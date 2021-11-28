//aba
// ./ex4a1 fifo0 fifo1 fifo2 fifo3

/*
  NOTE FOR ALL PROGRAMS
    - there are a lot of perrors
    - id like to send to function a string and then it will print <string> failed and then exit
*/

// --------include section------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>

#include <sys/stat.h> //named pipe

// --------const section------------------------

const int ARR_SIZE = 10000;
const int MAX_INUM = 1000;
const int ARGC_SIZE = 5;
const int NUM_OF_GEN = 3;
const int START = 1;
const int END = -1;

// --------struct section------------------------

//TODO: CHANGE TO RECEIVE ID (1, 2, OR 3) INSTEAD OF PID)
struct Data {
	pid_t _cpid; //child pid
	int _prime; //
};

// --------protoype section------------------------

void fill_array(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file, pid_t child[]);
void find_data(int arr[], int &counter, int &max, int &min);
void print_data(int arr[]);
void reset_arr(int arr[], int size_arr);
void check_argv(int argc );
bool prime(int num);

// --------main section------------------------

int main(int argc, char *argv[])
{
  //TODO: chaeck if 1 2 3 processes are connected to fifo
  pid_t ch_pid[NUM_OF_GEN];
	int child, index;

  check_argv(argc);
  if(mkfifo(argv[1], S_IFIFO | 0644) == -1 ||
	  mkfifo(argv[2], S_IFIFO | 0644) == -1 ||
	  mkfifo(argv[3], S_IFIFO | 0644) == -1 ||
	  mkfifo(argv[4], S_IFIFO | 0644) == -1)
  {
  	puts("mkfifo error");
  	exit(EXIT_FAILURE);
  }


  //TODO: add error if cannot open
  FILE *input_file = fopen(argv[1] ,"r");
  FILE *fifo1_file = fopen(argv[2] ,"w");
  FILE *fifo2_file = fopen(argv[3] ,"w");
  FILE *fifo3_file = fopen(argv[4] ,"w");
  //waits for numbers from all children in order to start(and tell them to start)

	for(index = 1; index <= NUM_OF_GEN; index++)
	{
			fscanf(input_file, "%d", &child);
			if(child == 1 || child == 2 || child == 3) //maybe without checking and just reading 3 times?
				continue;
			else
				printf("i received %d", child);
	}

	fprintf(fifo1_file, "%d", START);
	fprintf(fifo2_file, "%d", START);
	fprintf(fifo2_file, "%d", START);

  fill_array(input_file, fifo1_file, fifo2_file, fifo3_file, ch_pid);
  //fills array
  //sends back to sender number of times it received that prime
  //sends message to yazranim when the array is filled to sends -1?
  //prints number of different primes received

  //TODO: close fifo
  return EXIT_SUCCESS;
}





//=================================================================================

//-------------------------------------------------
// gets pipe of all children and the pid array
void fill_array(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file, pid_t child[])
{
	int primes_count[ARR_SIZE]; //count in each index the number of times father receive this number
	int filled = 0;
	struct Data data;
	reset_arr(primes_count, ARR_SIZE);


	while(filled < ARR_SIZE) // while EOF ?
	{
    //read process id and num
		fscanf(input_file, "%d %d", &data._cpid, &data._prime); //QUESTION: dont want to read into struct?? not possible in fifo??

		//check which child sent the number depend on the pid
		//send the counter of the prime num
		if (data._cpid == child[0]) //QUESTION: child is currently empty no?
		{
		  fprintf(fifo1_file, " %d", primes_count[data._prime]);
		  fflush(fifo1_file); //TODO: maybe we can put this in one place instead of 3
		}
		else if (data._cpid == child[1])
		{
		  fprintf(fifo2_file, " %d", primes_count[data._prime]);
		  fflush(fifo2_file);
		}
		else if (data._cpid == child[2])
		{
		  fprintf(fifo3_file, " %d", primes_count[data._prime]);
		  fflush(fifo3_file);
		}
		primes_count[data._prime]++;	//adds to counter
		filled++;					//increases fill number
	}

	//kills children
    fprintf(fifo1_file, " %d", END);
    fprintf(fifo2_file, " %d", END);
    fprintf(fifo3_file, " %d", END);

	//prints number of different primes, max and min received
	print_data(primes_count);

}

//-------------------------------------------------

void print_data(int arr[])
{
	int counter = 0, max = 2, min = MAX_INUM;
	find_data(arr, counter, max, min);

	printf("The number of different primes received is: %d\n", counter);
	printf("The max prime is: %d. The min primes is: %d", max, min);
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
	{
		arr[i] = 0;
	}
}


//function checks that both input file and output file names are
//given in argument vector
void check_argv(int argc )
{
	if(argc != ARGC_SIZE)
	{
		printf("Error! Incorrect number of arguments.\n");
		exit(EXIT_FAILURE);
	}
}
