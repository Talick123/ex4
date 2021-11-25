/*

	File: ex3b.c
	Collecting Prime Numbers from Children Processes
	=====================================================================
	Written by: Tali Kalev, ID:208629691, Login: talikal
		    		and	Noga Levy, ID:315260927, Login: levyno

	In this program, the parent program creates 3 children processes.
	Each child generates random numbers between 2 and 1000. Each time
	a primes number is generated, the child writes through the same pipe
	to their father their process id and the primes number (via struct).
	The father reads the prime numbers it receives, and returns to the
	specific child it got it from (via a personal pipe to that child,
	the amount of times it received the number. When the father receives
	1000 prime numbers, it sends a signal to kill all its children and
	prints how many different primes numbers it received. Before the
	child processes end, they print the number of times they sent
	their father a prime number it received for the first time.

	Compile: gcc -Wall ex3b.c -o ex3b

	Run: ./ex3b

	Input: No input

	Output: The number of new primes each child process sent, and the number
					of different primes the father process received.
		for Example:
				Child process 2123521 sent 34 new primes
				Child process 2123523 sent 9 new primes
				Child process 2123522 sent 111 new primes
				The number of different integers received is: 154

*/



// --------include section------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>

// ----------- const section ---------------------

const int ARR_SIZE = 1000;
const int NUM_OF_CHILDREN = 3;
const int SEED = 17;
const int NEW_PRIME = 0;

// ----------- enum and struct section ---------------------
enum CHILDREN { CHILD1, CHILD2, CHILD3};

struct Data {
	pid_t _cpid; //child pid
	int _prime; //
};

// ----------- global variables section --------------
int counter = 0;

// -------prototype section-----------------------

void create_children();
void handle_child(int pipe_fd1[], int pipe_r[]);
bool prime(int num);
void handle_father(int pipe_fd1[], int pipe_fd2[], int pipe_fd3[], int pipe_fd4[], pid_t child[]);
int count_primes(int arr[]);
void catch_sigterm(int signum);
void reset_arr(int arr[], int size_arr);

//---------main section---------------------------f

int main(int argc, char *argv[])
{
	srand(SEED); //turn 17 into const global seed
	create_children();

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void create_children()
{
	pid_t ch_pid[NUM_OF_CHILDREN]; //create array of processes
	int i;
	int pipe_fd1[2], //pipe for all children to write to parent
	 		pipe_fd2[2], //pipe for parent to write child1
			pipe_fd3[2], //pipe for parent to write child2
			pipe_fd4[2]; //pipe for parent to write child3

	//create pipes and handle eroor
	if (pipe(pipe_fd1) == -1 || pipe(pipe_fd2) == -1 ||
		pipe(pipe_fd3) == -1 || pipe(pipe_fd4) == -1)
	{
 	 	perror("cannot open pipe");
  	exit(EXIT_FAILURE) ;
	}

	// create 3 children
	for(i = 0; i < NUM_OF_CHILDREN; i++)
	{
		ch_pid[i] = fork(); //create child process

		if(ch_pid[i] < 0) // handle error in fork()
		{
			perror("Cannot fork()");
			exit (EXIT_FAILURE);
		}

		if(ch_pid[i] == 0) //if child
		{
			//each child gets different pipe to read from parent
			if(i == CHILD1)
				handle_child(pipe_fd1, pipe_fd2);

			else if(i == CHILD2)
				handle_child(pipe_fd1, pipe_fd3);

			else
				handle_child(pipe_fd1, pipe_fd4);
		}
	}
	handle_father(pipe_fd1, pipe_fd2, pipe_fd3, pipe_fd4, ch_pid);
}

//-------------------------------------------------

//all children write through the same pipe
//children read through 3 seperate pipes?
//gets pipe_fd1[] = pipe to write parent
//gets pipe_r[] = pipe to read from parent
void handle_child(int pipe_fd1[], int pipe_r[])
{
	signal(SIGTERM, catch_sigterm);
	struct Data data;
	data._cpid = getpid();
	int num;

	close(pipe_fd1[0]); //close pipe for reading
	close(pipe_r[1]); //close pipe for writing


	while(true)
	{
		num = rand()%(ARR_SIZE -1) + 2; //randomize num between 2 to 1000

		if(prime(num))
		{
			//send to father num + getpid() using pipe_fd1
			data._prime = num; // save prime num in struct
			// write to pipe the data (pid and prime num)
			write(pipe_fd1[1], &data, sizeof(struct Data));
			read(pipe_r[0], &num, sizeof(int));
		}
		if (num == NEW_PRIME)
			counter++;

	}

	//close all
	close(pipe_fd1[1]);
	close(pipe_r[0]);
}

//-------------------------------------------------
//gets integer and check if is prime
bool prime(int num)
{
	int i;
	for(i = 2; i*i < num; i++)
	{
		if(num % i == 0)
			return false;
	}
	return true;
}

//-------------------------------------------------
//catch signal of SIGTERM
void catch_sigterm(int signum)
{
	signal(SIGTERM, catch_sigterm);

	printf("Child process %d sent %d new primes\n", getpid(), counter);
	exit(EXIT_SUCCESS);
}

//-------------------------------------------------
// gets pipe of all children and the pid array
void handle_father(int pipe_fd1[], int pipe_fd2[], int pipe_fd3[], int pipe_fd4[], pid_t child[])
{
	int primes_count[ARR_SIZE]; //count in each index the number of times father receive this number
	int filled = 0, index;
	struct Data data;

	reset_arr(primes_count, ARR_SIZE);

	close(pipe_fd1[1]); //close for reading
	close(pipe_fd2[0]); //close for writing
	close(pipe_fd3[0]); //close for writing
	close(pipe_fd4[0]); //close for writing

	while(filled < ARR_SIZE)
	{
		//read number
		//sends prime count to child accordingly
		//enters into primes
		//increases prime count
		//increases filled counter

		// read from children pipe to get the data (prime number and child pid)
		//read(pipe_fd1[0], data, sizeof(struct Data));

		read(pipe_fd1[0], &data, sizeof(struct Data));

		//check which child sent the number depend on the pid
		//send the counter of the prime num
		if (data._cpid == child[0])
			write(pipe_fd2[1], &primes_count[data._prime], sizeof(int));
		else if (data._cpid == child[1])
			write(pipe_fd3[1], &primes_count[data._prime], sizeof(int));
		else if (data._cpid == child[2])
			write(pipe_fd4[1], &primes_count[data._prime], sizeof(int));

		primes_count[data._prime]++;	//adds to counter
		filled++;					//increases fill number
	}
	//kills children

	for(index = 0; index < NUM_OF_CHILDREN; index++)
		kill(child[index], SIGTERM);

	for(index = 0; index < NUM_OF_CHILDREN; index++)
		wait(NULL);


	//close all
	close(pipe_fd1[0]);
	close(pipe_fd2[1]);
	close(pipe_fd3[1]);
	close(pipe_fd4[1]);

	printf("The number of different integers received is: %d\n",
				 count_primes(primes_count));
}

//-------------------------------------------------
//
int count_primes(int arr[])
{
	int index, counter = 0;
	//start on i=2 - we can be sure that 0 and 1 is empty
	for(index = 2; index < ARR_SIZE; index++)
		if(arr[index] != 0)
			counter++;

	return counter;
}


//-------------------------------------------------

void reset_arr(int arr[], int size_arr)
{
	int i;
	for(i = 0; i < size_arr; i++)
	{
		arr[i] = 0;
	}
}
