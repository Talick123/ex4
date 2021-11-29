//prints also min and max prime values

/*
  NOTE FOR ALL PROGRAMS
    - there are a lot of perrors
    - id like to send to function a string and then it will print <string> failed and then exit
*/

/*
  NOTES FOR THIS SPECIFIC FILE
  - there is no mention of releasing queue, i think we have to add at the end when kids die
*/
// --------include section------------------------

#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
#include <unistd.h> //for sleep

// --------const section------------------------

const int ARR_SIZE = 100;
const int MAX_INUM = 1000;
const int NUM_OF_GEN = 3;
const int START = 1;
const int END = -1;

// --------struct section------------------------


struct Data {
	pid_t _cpid; //child pid
	int _prime; //
  int _status;
};

struct Msgbuf{
  long _type;
  struct Data _data;
};

// --------protoype section------------------------

void fill_array(int msqid, pid_t ch_pid[], struct Msgbuf *msg);
void find_data(int arr[], int *counter, int *max, int *min);
void print_data(int arr[]);
void reset_arr(int arr[], int size_arr);
void perror_and_exit(char *action);

// --------main section------------------------

int main()
{
	struct Msgbuf msg;
	pid_t ch_pid[NUM_OF_GEN];
	int msqid, index;
	key_t key;


	//creating external id for message queue
	if((key = ftok(".",'4')) == -1)
		perror_and_exit("ftok failed");

	//creating internal id for message queue
	if((msqid = msgget(key, 0600 | IPC_CREAT | IPC_EXCL)) == -1 && errno != EEXIST)
		perror_and_exit("msgget failed");

	printf("waiting to receive from children\n");
	//receiving 3 ones from children

	for(index = 0; index < NUM_OF_GEN; index++)
	{
		printf("reading from child\n");
		if(msgrcv(msqid, &msg, sizeof(struct Data), 1, 0) == -1)
			perror_and_exit("msgrcv");

		printf("checking child pid\n");
		if(msg._data._status == START)
		  ch_pid[index] = msg._data._cpid;
	}
	printf("msqid is: %d\n",msqid);
	sleep(5);
	/*
	printf("reading from child\n");
		if(msgrcv(msqid, &msg, sizeof(struct Data), 1, 0) == -1)
		{
			printf("msqid is: %d\n",msqid);
			printf("why\n");
			perror_and_exit("msgrcv");
		}
	printf("reading from child\n");
		if(msgrcv(msqid, &msg, sizeof(struct Data), 1, 0) == -1)
			perror_and_exit("msgrcv");
	printf("reading from child\n");
		if(msgrcv(msqid, &msg, sizeof(struct Data), 1, 0) == -1)
			perror_and_exit("msgrcv");
*/
	for(index = 0; index < NUM_OF_GEN; index++)
	{
		printf("SENDING TO %d\n",ch_pid[index]);
		kill(ch_pid[index], SIGUSR1); //each child has a sigusr1 catcher
	}

	printf("SENT SIGNAL\n");
	//starts to fill
	fill_array(msqid, ch_pid, &msg);

	return EXIT_SUCCESS;
}



//-------------------------------------------------
// gets pipe of all children and the pid array
void fill_array(int msqid, pid_t ch_pid[], struct Msgbuf *msg)
{
	printf("In fill array\n");
	int primes_count[ARR_SIZE]; //count in each index the number of times father receive this number
	int filled = 0, index;

	reset_arr(primes_count, ARR_SIZE);


	while(filled < ARR_SIZE)
	{
		printf("reading prime number\n");
		//reads from queue prime
		if(msgrcv(msqid, msg, sizeof(struct Data), 1, 0) == -1) //&msg??
			perror_and_exit("msgrcv failed");

		printf("i received %d\n",(*msg)._data._prime);
		printf("preparing to send back\n");
		(*msg)._type = (*msg)._data._cpid; //gets ready to send to proper child
		(*msg)._data._status = primes_count[(*msg)._data._prime]; //prepares counter

		printf("sending back\n");
		//sends to child number of times it received that prime
		if((msgsnd(msqid, msg, sizeof(struct Data), 0)) == -1)
			perror_and_exit("msgsnd failed");

		primes_count[(*msg)._data._prime]++;	//adds to counter
		filled++;				//increases fill number
		printf("filled is %d\n",filled);
	}


	//kills children
	printf("about to murder children\n");
	(*msg)._data._status = END;
	for(index = 0; index < NUM_OF_GEN; index++)
	{
		msg->_type = ch_pid[index]; //sending to proper child
		if(msgsnd(msqid, msg, sizeof(struct Data), 0) == -1)
			perror_and_exit("msgsnd failed");
	}
	sleep(3);
	//prints number of different primes, max and min received
	print_data(primes_count);
}

//-------------------------------------------------

void print_data(int arr[])
{
	int counter = 0, max = 2, min = MAX_INUM;
	find_data(arr, &counter, &max, &min);

	printf("The number of different primes received is: %d\n", counter);
	printf("The max prime is: %d. The min primes is: %d\n", max, min);
}

//-------------------------------------------------


void find_data(int arr[], int *counter, int *max, int *min)
{
	int index;
	//start on i=2 - we can be sure that 0 and 1 is empty
	for(index = 2; index < ARR_SIZE; index++)
	{
		if(arr[index] != 0)
			(*counter)++;
		if(arr[index] != 0 && index > *max)
			*max = index;
		if(arr[index] != 0 && index < *min)
			*min = index;
	}
}

//-------------------------------------------------

void reset_arr(int arr[], int size_arr)
{
	int i;
	for(i = 0; i < size_arr; i++)
		arr[i] = 0;
}

//-------------------------------------------------

void perror_and_exit(char *action)
{
	printf("about to perror\n");

	perror(action);
	exit(EXIT_FAILURE);
}
