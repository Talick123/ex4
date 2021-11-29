// ./ex4a1 fifo0 fifo1 fifo2 fifo3

/*
  NOTE FOR ALL PROGRAMS
    - there are a lot of perrors
    - id like to send to function a string and then it will print <string> failed and then exit
*/

//fills array
//sends back to sender number of times it received that prime
//sends message to yazranim when the array is filled to sends -1?
//prints number of different primes received

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

// --------const section------------------------

const int ARR_SIZE = 100;
const int MAX_INUM = 1000;
const int ARGC_SIZE = 5;
const int NUM_OF_GEN = 3;
const int START = 1;
const int END = -1;

// --------prototype section------------------------

void fill_array(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file, pid_t child[]);
void find_data(int arr[], int *counter, int *max, int *min);
void print_data(int arr[]);
void reset_arr(int arr[], int size_arr);
void check_argv(int argc );
bool prime(int num);
FILE * open_file(char* filename,  char *mode);
void send_start(FILE *fifo_name);


// --------main section------------------------

int main(int argc, char *argv[])
{
  	//TODO: chaeck if 1 2 3 processes are connected to fifo
 	int ch_id[NUM_OF_GEN];// = {-1, -1, -1};
	int child, ch_counter = 0;

  	check_argv(argc);
  	if((mkfifo(argv[1], S_IFIFO | 0644) == -1 ||
	  mkfifo(argv[2], S_IFIFO | 0644) == -1 ||
	  mkfifo(argv[3], S_IFIFO | 0644) == -1 ||
	  mkfifo(argv[4], S_IFIFO | 0644) == -1) && errno != EEXIST) // ?
  	{
  		puts("mkfifo error\n");
  		exit(EXIT_FAILURE);
  	}

	puts("before open files");
	FILE *input_file = open_file(argv[1] ,"r");
	FILE *fifo1_file = open_file(argv[2] ,"w");
	FILE *fifo2_file = open_file(argv[3] ,"w");
	FILE *fifo3_file = open_file(argv[4] ,"w");
	puts("after open file");
	//waits for numbers from all children in order to start(and tell them to start)

	puts("AHHHHHHHHHHHHHH ");
	/*
	while(ch_counter < NUM_OF_GEN) //T: dont love this way, is there a better way?
	{
		puts("wait for child");
		fscanf(input_file, " %d", &child); //probably nechsam WHHYYYYYYYY ???
		printf("hello %d\n", child);

		switch (child) {
			case 1:
			case 2:
			case 3:
				//ch_id[child -1] = child;
				ch_counter++;
				break;
			default:
				break;
		}
	}*/

	puts("wait for child");
	fscanf(input_file, " %d", &child);
	printf("hello %d\n", child);
	puts("wait for child");
	fscanf(input_file, " %d", &child);
	printf("hello %d\n", child);
	puts("wait for child");
	fscanf(input_file, " %d", &child);
	printf("hello %d\n", child);


	send_start(fifo1_file);
	send_start(fifo2_file);
	send_start(fifo3_file);

  fill_array(input_file, fifo1_file, fifo2_file, fifo3_file, ch_id);

  //close fifo
	fclose(input_file);
	fclose(fifo1_file);
	fclose(fifo2_file);
	fclose(fifo3_file);

  return EXIT_SUCCESS;
}

//-------------------------------------------------

// gets pipe of all children and the pid array
void fill_array(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file, int child[])
{
	int primes_count[ARR_SIZE]; //count in each index the number of times father receive this number
	int filled = 0;
	int prime = 0, child_id = 0;
	reset_arr(primes_count, ARR_SIZE);

	while(filled < ARR_SIZE) // while EOF ?
	{
    //read process id and num
		fscanf(input_file, " %d %d", &child_id, &prime);

		//check which child sent the number depend on the pid
		//send the counter of the prime num
		if (child_id == child[0])
		{
		  fprintf(fifo1_file, " %d\n", primes_count[prime]);  //QUESTION:newline?
		  fflush(fifo1_file);
		}
		else if (child_id == child[1])
		{
		  fprintf(fifo2_file, " %d\n", primes_count[prime]);
		  fflush(fifo2_file);
		}
		else if (child_id == child[2])
		{
		  fprintf(fifo3_file, " %d\n", primes_count[prime]);
		  fflush(fifo3_file);
		}
		primes_count[prime]++;	//adds to counter
		filled++;					//increases fill number
	}

	//kills children
  fprintf(fifo1_file, " %d\n", END); //newline??
  fprintf(fifo2_file, " %d\n", END);
  fprintf(fifo3_file, " %d\n", END);

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
	//ken ? (*MAX??? ) or just max
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
	printf("name %s mode= %s\n", filename, mode);

	if (fp == NULL)
	{
		printf("Error! cannot open %s  ", filename);
		exit (EXIT_FAILURE);
	}
    return fp;
}

//-------------------------------------------------

void send_start(FILE *fifo_name)
{
	fprintf(fifo_name, " %d\n", START); //T: newline? when writing to named pipe??
	fflush(fifo_name);
}
