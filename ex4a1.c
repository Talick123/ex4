//aba
// ./ex4a1 fifo0 fifo1 fifo2 fifo3

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
const int ARGC_SIZE = 5;
const int NUM_OF_GEN = 3;

// --------struct section------------------------

struct Data {
	pid_t _cpid; //child pid
	int _prime; //
};

// --------protoype section------------------------

void fill_array(FILE *input_file, FILE *fifo1_file, FILE *fifo2_file, FILE* fifo3_file, pid_t child[]);
int count_primes(int arr[]);
void reset_arr(int arr[], int size_arr);
void check_argv(int argc );
bool prime(int num);

// --------main section------------------------

int main(int argc, char *argv[])
{
  //TODO: chaeck if 1 2 3 processes are connected to fifo
  pid_t ch_pid[NUM_OF_GEN];

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
		fscanf(input_file, "%d %d", &data._cpid, &data._prime);

		//check which child sent the number depend on the pid
		//send the counter of the prime num
		if (data._cpid == child[0])
		{
		  fprintf(fifo1_file, " %d", primes_count[data._prime]);
		  fflush(fifo1_file);
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
    fprintf(fifo1_file, " %d", -1);
    fprintf(fifo2_file, " %d", -1);
    fprintf(fifo3_file, " %d", -1);

	//add min max primes
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
