
/*
  NOTE FOR ALL PROGRAMS
    - there are a lot of perrors
    - id like to send to function a string and then it will print <string> failed and then exit
*/

//receives via argument vector the number to send to aba
//sends to aba starting number
//waits for number back in order to start
//randomizes numbers in an infinite loop
//each time there is prime, sends to first program through named pipe
//when ending message received, prints how many new primes sent
//ends
// --------include section------------------------

#include <string.h>//for strcat
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h> //named pipe

// --------const section------------------------

const int ARR_SIZE = 1000;
const int ARGC_SIZE = 3;
const int START = 1;
const int END = -1;

// --------struct section------------------------

struct Data {
	pid_t _cpid; //child pid T: change to child num or something like that?
	int _prime; //
};

// --------prototype section------------------------

void handle_child(FILE *fifo_w, FILE *fifo_r);
bool prime(int num);
void check_argv(int argc);
void check(int status,int prime, int &max, int &counter);
void print_and_end(int max, int counter);

// --------main section------------------------

int main(int argc, char *argv[])
{
	int status;
	check_argv(argc);
	srand(atoi(argv[2]));
	//TODO: add error if cannot open
	FILE *input_file = fopen(argv[1] ,"r");
	//char *fifo_name = strcat("fifo", argv[2]); //TODO: switch to different format
	//i think this is the proper way: (i might be wrong)
	char fifo_name[5] = "fifo"; //5 as a const?
	strcat(fifo_name, argv[2]);
	FILE *fifo_file = fopen(fifo_name ,"w");



	fprintf(input_file, " %d", argv[2]); //sends to aba his number to say he is ready
	fscanf(fifo_file, "%d", &status); //reads command to start

	if(status = START)
		handle_child(input_file, fifo_file, atoi(argv[2]));

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void handle_child(FILE *fifo_w, FILE *fifo_r, int childnum)
{
	struct Data data;
	data._cpid = childnum;
	int status, num, max = 0, counter = 0; //start at 0 in case didnt get to send any

	while(true)
	{
		num = rand()%(ARR_SIZE -1) + 2; //randomize num between 2 to 1000

		if(prime(num))
		{
			//send to father num + getpid() using pipe_fd1
			data._prime = num; // save prime num in struct
			// write to pipe the data (pid and prime num)
			fprintf(fifo_w, " %d %d", data._cpid, data._prime ); //newline???
			fscanf(fifo_r," %d", &status);
			check(status, num, max, counter);
		}
	}
}

//-------------------------------------------------

//checks number received from aba and saves if new max or ends if necessary
void check(int status,int prime, int &max, int &counter)
{
	if(status == END)
		print_and_end(max, counter);

	if(status > counter)
	{
		counter = status;
		max = prime;
	}
}

//-------------------------------------------------

void print_and_end(int max, int counter)
{
		if(counter == 0)
			printf("Process %d sent %d primes\n", (int)getpid(), counter);
		else
			printf("Process %d sent the prime %d, %d times\n", (int)getpid(), max, counter);

		exit(EXIT_SUCCESS);
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

void check_argv(int argc )
{
	if(argc != ARGC_SIZE)
	{
		printf("Error! Incorrect number of arguments.\n");
		exit(EXIT_FAILURE);
	}
}
