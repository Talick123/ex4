


// --------include section------------------------

#include <string.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>

//named pipe
#include <sys/stat.h>

const int ARR_SIZE = 1000;

struct Data {
	pid_t _cpid; //child pid
	int _prime; //
};

void handle_child(FILE *fifo_w, FILE *fifo_r);
bool prime(int num);

int main(int argc, char *argv[])
{
	//TODO: add error if cannot open
	//check 3 argc
	//s
	FILE *input_file = fopen(argv[1] ,"r");
	char *fifo_name = strcat("fifo", argv[2]);
	FILE *fifo_file = fopen(fifo_name ,"w");

	handle_child(input_file, fifo_file);
	//receives via argument vector the number to send to aba
	//sends to aba starting number
	//waits for number back in order to start
	//randomizes numbers in an infinite loop
	//each time there is prime, sends to first program through named pipe
	//when ending message received, prints how many new primes sent
	//ends
	return EXIT_SUCCESS;
}




//all children write through the same pipe
//children read through 3 seperate pipes?
//gets pipe_fd1[] = pipe to write parent
//gets pipe_r[] = pipe to read from parent
void handle_child(FILE *fifo_w, FILE *fifo_r)
{
	struct Data data;
	data._cpid = getpid();
	int num;


	while(true)
	{
		num = rand()%(ARR_SIZE -1) + 2; //randomize num between 2 to 1000

		if(prime(num))
		{
			//send to father num + getpid() using pipe_fd1
			data._prime = num; // save prime num in struct
			// write to pipe the data (pid and prime num)
			fprintf(fifo_w, " %d %d", data._cpid, data._prime );
			fscanf(fifo_r," %d", &num);
		}
		/*if (num == NEW_PRIME)
			counter++;
			*/
	}
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
