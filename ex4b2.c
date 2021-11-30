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

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> //for pause
#include <sys/wait.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>

// --------const section------------------------

const int ARR_SIZE = 10000;
const int ARGC_SIZE = 2;
const int START = 1;
const int END = -1;
const int ALLOWED_TYPE = 1;

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

// --------prototype section----------------------

void catch_sigusr1(int signum);
void handle_child(int msqid, struct Msgbuf *msg);
bool prime(int num);
void check_argv(int argc);
void check(int status,int prime, int *max, int *counter);
void print_and_end(int *max, int *counter);
void perror_and_exit(char *action);

// --------main section------------------------

int main(int argc, char *argv[])
{
	struct Msgbuf msg;
	int msqid;
	key_t key;

	signal(SIGUSR1, catch_sigusr1);

	check_argv(argc);
	srand(atoi(argv[1]));

	//creating external id for message queue
	if((key = ftok(".",'4')) == -1)
		perror_and_exit("ftok");

	//creating internal id for message queue
	if((msqid = msgget(key,0)) == -1 && errno != EEXIST)
		perror_and_exit("msgget");

	//sending to aba that hes ready to start
	msg._data._status = START;
	msg._type = ALLOWED_TYPE;
	msg._data._cpid = getpid();
	if(msgsnd(msqid, &msg, sizeof(struct Data), 0) == -1)
		perror_and_exit("msgsnd");

	pause(); //waits for signal from father
	handle_child(msqid, &msg); //when signal received, continues to here

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void catch_sigusr1(int signum) {} //doesnt do anything when signal is caught

//-------------------------------------------------

void handle_child(int msqid, struct Msgbuf *msg)
{
	int num, max = 0, counter = 0; //start at 0 in case didnt get to send any

	while(true)
	{
		num = rand()%(ARR_SIZE -1) + 2; //randomize num between 2 to 1000

		if(prime(num))
		{

			(*msg)._data._prime = num; // save prime num in struct
			(*msg)._type = ALLOWED_TYPE;

			//sends to father
			if(msgsnd(msqid, msg, sizeof(struct Data), 0) == -1)
				perror_and_exit("msgsnd");


			//reads from father
			if(msgrcv(msqid, msg, sizeof(struct Data), getpid() , 0) == -1)
				perror_and_exit("msgrcv");

			//checks data received
			check((*msg)._data._status, num, &max, &counter);
		}
	}
}

//-------------------------------------------------

//checks number received from aba and saves if new max or ends if necessary
void check(int status,int prime, int *max, int *counter)
{
	if(status == END)
		print_and_end(max, counter);

	if(status > *counter)
	{
		*counter = status;
		*max = prime;
	}
}

//-------------------------------------------------

void print_and_end(int *max, int *counter)
{
	if(counter == 0)
		printf("Process %d didnt send any prime, max number of times\n",
			(int)getpid());
	else
		printf("Process %d sent the prime %d, %d times\n",
			(int)getpid(), *max, *counter);

	exit(EXIT_SUCCESS);
}

//-------------------------------------------------

//gets integer and check if is prime
bool prime(int num)
{
	int i;
	for(i = 2; i*i <= num; i++)
		if(num % i == 0)
			return false;

	return true;
}

//-------------------------------------------------

void check_argv(int argc)
{
	if(argc != ARGC_SIZE)
		perror_and_exit("Error! Incorrect number of arguments");
}

//-------------------------------------------------

void perror_and_exit(char *action)
{
	perror(action);
	exit(EXIT_FAILURE);
}
