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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>

// --------const section------------------------

const int ARR_SIZE = 10000;
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

void fill_array(int msqid, pid_t ch_pid[], struct Msgbuf &msg);
void find_data(int arr[], int &counter, int &max, int &min);
void print_data(int arr[]);
void reset_arr(int arr[], int size_arr);

// --------main section------------------------

int main()
{
  struct Msgbuf msg;
  pid_t ch_pid[NUM_OF_GEN];
  int msqid, index;
  key_t key;

  //creating external id for message queue
  if((key = ftok(".",'4')) == -1)
  {
    perror("ftok failed\n");
    exit(EXIT_FAILURE);
  }

  //creating internal id for message queue
  if((msqid = msgget(key, 0600 | IPC_CREAT | IPC_EXCL)) == -1)
  {
    perror("msgget failed\n");
    exit(EXIT_FAILURE);
  }

  //receiving 3 ones from children
  for(index = 0; index < NUM_OF_GEN; index++)
  {
    if(msgrv(msqid, &msg, sizeof(struct Data), 1, 0) == -1)
    {
      perror("msgrcv failed\n");
      exit(EXIT_SUCCESS);
    }
    if(msg._data._status == START)
    {
      ch_pid[index] = msg._data._cpid;
      continue;
    }
    else
      printf("i received %d\n", msg._data._status);
  }

  for(index = 0; index < NUM_OF_GEN; index++)
    kill(ch_pid[index], SIGUSR1); //each child has a sigusr1 catcher


  //starts to fill
  fill_array(msqid, ch_pid, msg);
  return EXIT_SUCCESS;
}



//-------------------------------------------------
// gets pipe of all children and the pid array
void fill_array(int msqid, pid_t ch_pid[], struct Msgbuf &msg)
{
	int primes_count[ARR_SIZE]; //count in each index the number of times father receive this number
	int filled = 0, index;
	struct Data data;
	reset_arr(primes_count, ARR_SIZE);


	while(filled < ARR_SIZE)
	{
    //reads from queue prime
    if(msgrcv(msqid, &msg, sizeof(struct Data), 1, 0) == -1)
    {
      perror("msgrcv failed\n");
      exit(EXIT_SUCCESS);
    }
    msg._type = msg._data._cpid; //gets ready to send to proper child
    msg._data._status = primes_count[msg._data._prime]; //prepares counter

    //sends to child number of times it received that prime
    if(msgsnd(msqid, &msg, sizeof(struct Data), 0) == -1)
    {
      perror("msgsnd failed\n");
      exit(EXIT_FAILURE);
    }

		primes_count[msg._data._prime]++;	//adds to counter
		filled++;					//increases fill number
	}

	//kills children
  msg._data._status = END;
  for(index = 1; index <= NUM_OF_GEN; index++)
  {
    msg._type = ch_pid[index]; //sending to proper child
    if(msgsnd(msqid, &msg, sizeof(struct Data), 0) == -1)
    {
      perror("msgsnd failed\n");
      exit(EXIT_FAILURE);
    }
  }
	//prints number of different primes, max and min received
	print_data(primes_count);

}

//-------------------------------------------------

void print_data(int arr[])
{
	int counter = 0, max = 2, min = MAX_INUM;
	find_data(arr, counter, max, min);

	printf("The number of different primes received is: %d\n", counter);
	printf("The max prime is: %d. The min primes is: %d\n", max, min);
}

//-------------------------------------------------


void find_data(int arr[], int &counter, int &max, int &min)
{
	int index;
	//start on i=2 - we can be sure that 0 and 1 is empty
	for(index = 2; index < ARR_SIZE; index++)
	{
		if(arr[index] != 0)
			counter++;
		if(arr[index] != 0 && index > max)
			max = index;
		if(arr[index] != 0 && index < min)
			min = index;
	}
}


//-------------------------------------------------

void reset_arr(int arr[], int size_arr)
{
	int i;
	for(i = 0; i < size_arr; i++)
		arr[i] = 0;
}
