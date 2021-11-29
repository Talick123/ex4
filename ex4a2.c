
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

const int ARR_SIZE = 100;
const int ARGC_SIZE = 3;
const int START = 1;
const int END = -1;

// --------prototype section------------------------

void handle_child(FILE *fifo_w, FILE *fifo_r, int childnum);
bool is_prime(int num);
void check_argv(int argc);
void check(int status,int prime, int *max, int *counter);
void print_and_end(int max, int counter);
FILE * open_file(char* filename,  char *mode);
void start_proc(FILE *input_file, FILE *fifo_file, int childnum);


// --------main section------------------------

int main(int argc, char *argv[])
{
	check_argv(argc);
	srand(atoi(argv[2]));

	char fifo_name[5] = "fifo"; //5 as a const?
	strcat(fifo_name, argv[2]);
	printf("before open file\n");
	FILE *input_file = open_file(argv[1] ,"w");
	FILE *fifo_file = open_file(fifo_name ,"r");
	printf("after open file\n");
	start_proc(input_file, fifo_file, atoi(argv[2]));

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void start_proc(FILE *input_file, FILE *fifo_file, int childnum)
{
	int status;
	//sleep(childnum);
	printf("k send from %d\n", childnum);
	fprintf(input_file, " %d\n", childnum); //sends to aba his number to say he is ready
	fflush(input_file);
	printf("after flush\n");
	fscanf(fifo_file, " %d", &status); //reads command to start

	if(status == START)
	{
		puts("ok start");
		handle_child(input_file, fifo_file, childnum);
	}
}

//-------------------------------------------------

void handle_child(FILE *fifo_w, FILE *fifo_r, int childnum)
{
	int status, num, max = 0, counter = 0; //start at 0 in case didnt get to send any

	while(true)
	{
		num = rand()%(ARR_SIZE -1) + 2; //randomize num between 2 to 1000

		if(is_prime(num))
		{
			//send to father num + getpid() using pipe_fd1
			// write to pipe the data (pid and prime num)
			fprintf(fifo_w, " %d %d\n", childnum, num ); //newline???
			fscanf(fifo_r," %d", &status);
			check(status, num, &max, &counter);
		}
	}
}

//-------------------------------------------------

//checks number received from aba and saves if new max or ends if necessary
void check(int status,int prime, int *max, int *counter)
{
	if(status == END)
		print_and_end((*max), (*counter));

	if(status > (*counter))
	{
		//ken? lo?
		(*counter) = status;
		(*max) = prime;
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
bool is_prime(int num)
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

//-------------------------------------------------

FILE * open_file(char* filename,  char *mode)
{
	FILE *fp = fopen(filename, mode);

	if (fp == NULL)
	{
		printf("Error! cannot open %s  ", filename);
		exit (EXIT_FAILURE);
	}
    return fp;
}
