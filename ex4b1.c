/*
File: ex4b1.c ex4b2.c
Generate and Collect Primes from Message Queue
=====================================================================
Written by: Tali Kalev, ID:208629691, Login: talikal
		and	Noga Levy, ID:315260927, Login: levyno

This program runs with 4 different processes. Three processes that generates
random numbers, when the number is prime the process sends it to main process via message queue.
And one process - the main process that collect the primes sent from the other processes
via message queue and insert them into array, when the main process gets 10000 primes,
alert to the 3 other process to end, prints the minimum prime, max prime and number of
different numbers in the array, close message queue, prints data and finishes.
when the other processes get the message from main process to end they print the
prime number they send the most to main process, and finish.

Compile: gcc -Wall ex4b1.c -o ex4b1
				 gcc -Wall ex4b2.c -o ex4b2
		 (ex4b1 = main process, ex4b2 = sub process)

Run: for start run the main process.
		Then, run 3 times the sub processes and send to the vector
		arguments the number of process (1-3):
				./ex4b1
				./ex4b2 1
				./ex4b2 2
				./ex4b2 3

Input: No Input

Output:
		From main process (ex4b1) = minimum prime, max prime and number of
		different numbers in the array.
		Example: The number of different primes received is: 168
						 The max prime is: 997. The min primes is: 2
		From sub process (ex4b2) = prime number they send the most to main process
		Example: Process 1101373 sent the prime 233, 14 times

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

const int ARR_SIZE = 10000;
const int MAX_INUM = 1000;
const int NUM_OF_GEN = 3;
const int START = 1;
const int END = -1;

// --------struct section------------------------


struct Data {
	pid_t _cpid; //child pid
	int _prime;
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
void delete_queue(int msqid);

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


	for(index = 0; index < NUM_OF_GEN; index++)
	{
		if(msgrcv(msqid, &msg, sizeof(struct Data), 1, 0) == -1)
			perror_and_exit("msgrcv");

		if(msg._data._status == START)
		  ch_pid[index] = msg._data._cpid;
	}

	for(index = 0; index < NUM_OF_GEN; index++)
		kill(ch_pid[index], SIGUSR1); //each child has a sigusr1 catcher

	//starts to fill
	fill_array(msqid, ch_pid, &msg);

	return EXIT_SUCCESS;
}



//-------------------------------------------------
// gets pipe of all children and the pid array
void fill_array(int msqid, pid_t ch_pid[], struct Msgbuf *msg)
{
	int primes_count[ARR_SIZE]; //count in each index the number of times father receive this number
	int filled = 0, index;

	reset_arr(primes_count, ARR_SIZE);

	while(filled < ARR_SIZE)
	{
		//reads from queue prime
		if(msgrcv(msqid, msg, sizeof(struct Data), 1, 0) == -1) //&msg??
			perror_and_exit("msgrcv failed");

		(*msg)._type = (*msg)._data._cpid; //gets ready to send to proper child
		(*msg)._data._status = primes_count[(*msg)._data._prime]; //prepares counter

		//sends to child number of times it received that prime
		if((msgsnd(msqid, msg, sizeof(struct Data), 0)) == -1)
			perror_and_exit("msgsnd failed");

		primes_count[(*msg)._data._prime]++;	//adds to counter
		filled++;				//increases fill number
	}


	//kills children
	(*msg)._data._status = END;
	for(index = 0; index < NUM_OF_GEN; index++)
	{
		msg->_type = ch_pid[index]; //sending to proper child
		if(msgsnd(msqid, msg, sizeof(struct Data), 0) == -1)
			perror_and_exit("msgsnd failed");
	}

	//prints number of different primes, max and min received
	print_data(primes_count);
	delete_queue(msqid);
}

//-------------------------------------------------

void print_data(int arr[])
{
	int counter = 0, max = 2, min = MAX_INUM;
	find_data(arr, &counter, &max, &min);

	printf("The number of different primes received is: %d\n", counter);
	printf("The max prime is: %d. The min prime is: %d\n", max, min);
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

//setting values in array to 0
void reset_arr(int arr[], int size_arr)
{
	int i;
	for(i = 0; i < size_arr; i++)
		arr[i] = 0;
}

//-------------------------------------------------

void perror_and_exit(char *action)
{
	perror(action);
	exit(EXIT_FAILURE);
}

//-------------------------------------------------

void delete_queue(int msqid)
{
	if(msgctl(msqid, IPC_RMID, NULL) == -1)
		perror_and_exit("msgctl failed");

	exit(EXIT_SUCCESS);
}
