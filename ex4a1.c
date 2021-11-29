// Run: ./ex4a1 fifo0 fifo1 fifo2 fifo3
/*
	File: ex4a1.c ex4a2.c
	Generate and Collect Primes from Named Pipe
	=====================================================================
	Written by: Tali Kalev, ID:208629691, Login: talikal
			and	Noga Levy, ID:315260927, Login: levyno

	This program runs with 4 different processes. Three processes that generates
	random numbers, when the number is prime the process send it to main process via named pipe.
	And one process - the main process that collect the primes send from the other processes
	via named pipe and insert them into array, when the main process gets 1000 primes,
	alert to the 3 other process to end, prints the minimum prime, max prime and number of
	different numbers in the array, close all named pipe and finish.
	when the other processes get the massage from main process to end they prints the
	prime number they send the most to main process, and finish.

	Compile: gcc -Wall ex4a1.c -o ex4a1
	         gcc -Wall ex4a2.c -o ex4a2
	     (ex4a1 = main process, ex4a2 = sub process)

	Run: for start run the main process with the 4 named pipe names.
	    Then, run 3 times the sub processes and send to the vector
	    arguments the main named pipe 'fifo0' and the number of process (1-3):
	        ./ex4a1 fifo0 fifo1 fifo2 fifo3
	        ./ex4a2 fifo0 1
	        ./ex4a2 fifo0 2
	        ./ex4a2 fifo0 3

	Input: No Input

	Output:
	    From main process (ex4a1) = minimum prime, max prime and number of
	    different numbers in the array.
	    Example: The number of different primes received is: 168
	             The max prime is: 997. The min primes is: 2
	    From sub process (ex4a2) = prime number they send the most to main process
	    Example: Process 1101373 sent the prime 233, 14 times
*/

// --------include section------------------------

#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>
#include <sys/stat.h> //named pipe

// --------const and enum section------------------------

const int ARR_SIZE = 1000;
const int MAX_INUM = 1000;
const int ARGC_SIZE = 5;
const int NUM_OF_GEN = 3;
const int START = 1;
const int END = -1;

enum childnum { CHILD_1 = 1 , CHILD_2, CHILD_3 };

// --------prototype section------------------------

void fill_array(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file);
void find_data(int arr[], int *counter, int *max, int *min);
void print_data(int arr[]);
void reset_arr(int arr[], int size_arr);
void check_argv(int argc );
bool prime(int num);
FILE * open_file(char* filename,  char *mode);
void send_start(FILE *fifo_name);
void write_int_to_fifo(FILE *fifo_file, int data);
void start_process(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file);
void end_process(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file);
void create_all_fifo(char *argv[]);

// --------main section------------------------

int main(int argc, char *argv[])
{
	check_argv(argc);

	create_all_fifo(argv);

	//open fifo files
	FILE *input_file = open_file(argv[1] ,"r");
	FILE *fifo1_file = open_file(argv[2] ,"w");
	FILE *fifo2_file = open_file(argv[3] ,"w");
	FILE *fifo3_file = open_file(argv[4] ,"w");

	start_process(input_file, fifo1_file, fifo2_file, fifo3_file);
	end_process(input_file, fifo1_file, fifo2_file, fifo3_file);

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void start_process(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file)
{
	int child, i;

	for(i = 0; i < NUM_OF_GEN; i++)
	{
		fscanf(input_file, " %d", &child);
	}

	send_start(fifo1_file);
	send_start(fifo2_file);
	send_start(fifo3_file);

	fill_array(input_file, fifo1_file, fifo2_file, fifo3_file);
}

//-------------------------------------------------

void end_process(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file)
{
	fclose(input_file);
	fclose(fifo1_file);
	fclose(fifo2_file);
	fclose(fifo3_file);
	//maybe unlink them?
}

//-------------------------------------------------

// gets pipe of all children and the pid array
void fill_array(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file)
{
	int primes_count[ARR_SIZE]; //count in each index the number of times father receive this number
	int filled = 0, prime = 0, child_id = 0;
	reset_arr(primes_count, ARR_SIZE);

	while(filled < ARR_SIZE)
	{
    	//read process id and num
		fscanf(input_file, " %d %d", &child_id, &prime);

		//check which child sent the number depend on the pid
		//send the counter of the prime num
		if (child_id == CHILD_1)
		{
		  	write_int_to_fifo(fifo1_file, primes_count[prime]);
		}
		else if (child_id == CHILD_2)
		{
		  	write_int_to_fifo(fifo2_file, primes_count[prime]);
		}
		else if (child_id == CHILD_3)
		{
		  	write_int_to_fifo(fifo3_file, primes_count[prime]);
		}

		primes_count[prime]++;	//adds to counter
		filled++;					//increases fill number
	}

	//kills children
	write_int_to_fifo(fifo1_file, END);
	write_int_to_fifo(fifo2_file, END);
	write_int_to_fifo(fifo3_file, END);

	sleep(1);
	print_data(primes_count); //prints number of different primes, max and min received
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
		if(arr[index] != 0 && index > (*max))
			(*max) = index;
		if(arr[index] != 0 && index < (*min))
			(*min) = index;
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

void check_argv(int argc )
{
	if(argc != ARGC_SIZE)
	{
		printf("Error! Incorrect number of arguments.\n");
		exit(EXIT_FAILURE);
	}
}

//-------------------------------------------------

FILE *open_file(char* filename,  char *mode)
{
	FILE *fp = fopen(filename, mode);

	if (fp == NULL)
	{
		printf("Error! cannot open %s  ", filename);
		exit (EXIT_FAILURE);
	}
    return fp;
}

//-------------------------------------------------

void create_all_fifo(char *argv[])
{
	//create fifo if not exist
  	if((mkfifo(argv[1], S_IFIFO | 0644) == -1 ||
	  mkfifo(argv[2], S_IFIFO | 0644) == -1 ||
	  mkfifo(argv[3], S_IFIFO | 0644) == -1 ||
	  mkfifo(argv[4], S_IFIFO | 0644) == -1) && errno != EEXIST)
  	{
		puts("mkfifo error\n");
  		exit(EXIT_FAILURE);
  	}
}

//-------------------------------------------------

void send_start(FILE *fifo_name)
{
	fprintf(fifo_name, " %d\n", START);
	fflush(fifo_name);
}

//-------------------------------------------------

void write_int_to_fifo(FILE *fifo_file, int data)
{
	fprintf(fifo_file, " %d\n", data);
	fflush(fifo_file);
}
