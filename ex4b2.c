//prints the value that was sent most often and number of times

/*
  NOTE FOR ALL PROGRAMS
    - there are a lot of perrors
    - id like to send to function a string and then it will print <string> failed and then exit
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

const int ARR_SIZE = 100;
const int ARGC_SIZE = 2;
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

	printf("msqid is: %d\n",msqid);
	printf("about to send to aba\n");
	//can put this in function
	//sending to aba that hes ready to start
	msg._data._status = START;
	msg._type = 1;
	msg._data._cpid = getpid();
	printf("MY PID IS: %d\n",getpid());
	if(msgsnd(msqid, &msg, sizeof(struct Data), 0) == -1)
	{
		printf("tried to send and didnt succeed\n");
		perror_and_exit("msgsnd");
	}

	printf("msqid is: %d\n",msqid);
	printf("I SENT TO ABA\n");
	pause(); //waits for signal from father
	printf("I RECEIVED FROM ABA\n");
	handle_child(msqid, &msg); //when signal received, continues to here

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void catch_sigusr1(int signum) {} //doesnt do anything when signal is caught

//-------------------------------------------------

void handle_child(int msqid, struct Msgbuf *msg)
{
	printf("about to handle child\n");
	int num, max = 0, counter = 0; //start at 0 in case didnt get to send any

	while(true)
	{
		num = rand()%(ARR_SIZE -1) + 2; //randomize num between 2 to 1000
		//printf("number generated is %d\n",num);
		if(prime(num))
		{
			//printf("prime generated is %d\n",num);
			(*msg)._data._prime = num; // save prime num in struct
			(*msg)._type = 1;
			//printf("about to send to father\n");
			//sends to father
			if(msgsnd(msqid, msg, sizeof(struct Data), 0) == -1)
				perror_and_exit("msgsnd");

			//printf("reading from father\n");
			//reads from father
			if(msgrcv(msqid, msg, sizeof(struct Data), getpid() , 0) == -1)
				perror_and_exit("msgrcv");

			//printf("checking data\n");
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
			printf("Process %d didnt send any prime, max number of times\n", (int)getpid());
		else
			printf("Process %d sent the prime %d, %d times\n", (int)getpid(), *max, *counter);

		exit(EXIT_SUCCESS);
}

//-------------------------------------------------
//gets integer and check if is prime
bool prime(int num)
{
	int i;
	for(i = 2; i*i <= num; i++)
	{
		if(num % i == 0)
			return false;
	}
	return true;
}

//-------------------------------------------------

void check_argv(int argc)
{
	if(argc != ARGC_SIZE)
	{
		printf("Error! Incorrect number of arguments.\n");
		exit(EXIT_FAILURE);
	}
}

//-------------------------------------------------

void perror_and_exit(char *action)
{
	printf("about to perror\n");
	strcat(action, "failed");

	perror(action);
	exit(EXIT_FAILURE);
}
