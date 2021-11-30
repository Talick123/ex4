/*

File: ex4c1.c ex4c2.c ex4c3.c
Interactions Between Registry, Application and Customer
=====================================================================
Written by: Tali Kalev, ID:208629691, Login: talikal
		and	Noga Levy, ID:315260927, Login: levyno

This program allows a customer to register via a registry server and then
use the tools given in the application server via message queue.

In the Registry Server: customers and the application server can request
via the message queue to either be added to the registry, check if they
are in the registry already and to be removed from the registry.

In the Application server: customers can request via message Queue to check
if an integer is prime or if a string is a palindrome. Before completing these
requests from the customer, the application server checks with the registry
server if the customer exists already in the registry.

The customer begins by registering via message queue. After that, the customer
reads from the user either an integer or a string and checks whether it is
a prime or palindrome respectively via message queue with the application
server. The customer prints the result on the screen. When the customer is done
it requests to remove itself from the registry.

Compile: gcc -Wall ex4c1.c -o ex4c1
         gcc -Wall ex4c2.c -o ex4c2
				 gcc -Wall ex4c3.c -o ex4c3
     (ex4c1 = registry server, ex4c2 = application server, ex4c3.c = customer)

Run: First, run registry server, then application server and then as many
		customers as desired (here just 1):
			./ex4c1
			./ex4c2
			./ex4c3

Input: The registry server and application server require no input.
			The customer requires input of either 'n' 's' or 'e'. If 'n' then
			an integer. If 's' then a string ('e' removes from registry and exits).
			Example:
				n
				7

Output: The registry server and application server have no output.
				The customer will print the result of its request depending on the
				user input. If the input is as above, the output will be:
				Example:
				7 is a prime.
*/
// --------include section------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>

// --------const and enum section------------------

enum Requests {ADD, INARR, REMOVE};
enum Add_answers {ADDED, EXISTS, FULL};
enum Exists_answers {DOESNTEXIST, DOESEXIST};
const int ARR_SIZE = 100;
int msqid;

// --------struct section------------------------

struct Data {
	pid_t _cpid; //child pid
	pid_t _return; //pid of messenger
	int _status;
};

struct Msgbuf{
	long _type;
	struct Data _data;
};
// --------prototype section---------------------

void catch_int(int signum);
void read_requests(int msqid, struct Msgbuf *msg);
int add_to_arr(pid_t arr[], pid_t pid, int *filled);
int in_arr(pid_t arr[], pid_t pid, int *filled);
void remove_from_arr(pid_t arr[], pid_t pid, int *filled);
bool exists_in_arr(pid_t arr[], pid_t pid, int *filled);
void perror_and_exit(char *action);

// --------main section--------------------------

int main()
{
	struct Msgbuf msg;
	key_t key;

	signal(SIGINT, catch_int);

	//creating external id for message queue
	if((key = ftok(".",'c')) == -1)
		perror_and_exit("ftok failed");

	//creating internal id for message queue
	if((msqid = msgget(key, 0600 | IPC_CREAT | IPC_EXCL)) == -1 && errno != EEXIST)
		perror_and_exit("msgget failed");

	read_requests(msqid, &msg);

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void catch_int(int signum)
{
	if(msgctl(msqid, IPC_RMID, NULL) == -1)
		perror_and_exit("msgctl failed");

	exit(EXIT_SUCCESS);
}

//-------------------------------------------------

void read_requests(int msqid, struct Msgbuf *msg)
{
  pid_t arr[ARR_SIZE];
  int status, filled = 0;

  while(true)
  {
	//read request
    if(msgrcv(msqid, msg, sizeof(struct Data), 1, 0) == -1)
		perror_and_exit("msgrcv failed");

    switch((*msg)._data._status)
    {
	  case ADD:
		//checks if exist in array, or if full,  adds if it can
		status = add_to_arr(arr, (*msg)._data._cpid, &filled);
		break;
	  case INARR:
		//checks if it exists in array
		status = in_arr(arr, (*msg)._data._cpid, &filled);
		break;
	  case REMOVE:
		//removes from array
		remove_from_arr(arr, (*msg)._data._cpid, &filled);
		break;
    }

	//sending back status if not remove
	if((*msg)._data._status != REMOVE)
	{
		(*msg)._type = (*msg)._data._return; //prepares to send status back to sender
		(*msg)._data._status = status;

		//sends status to sender
		if(msgsnd(msqid, msg, sizeof(struct Data), 0) == -1)
			perror_and_exit("msgsnd failed");
	}
  }
}

//-------------------------------------------------

int add_to_arr(pid_t arr[], pid_t pid, int *filled)
{

  //checks if array is already full
  if(*filled == (ARR_SIZE - 1))
    return FULL;

  //checks in filled part of array if already exists
  if(exists_in_arr(arr, pid, filled))
      return EXISTS;

  //if does not exist, adds and returns
  arr[(*filled)++] = pid;
  return ADDED;

}

//-------------------------------------------------

int in_arr(pid_t arr[], pid_t pid, int *filled)
{
  if(exists_in_arr(arr, pid, filled))
    return DOESEXIST;

  return DOESNTEXIST;
}

//-------------------------------------------------

//function removed pid from array
void remove_from_arr(pid_t arr[], pid_t pid, int *filled)
{
	int index, place;

	for(index = 0; index < *filled; index++)
	{
		if(arr[index] == pid) //if found
		{
			for(place = index; place < (*filled) - 1; place++)
				arr[place] = arr[place + 1]; //move values backwards in array

			arr[place] = 0;
			--(*filled); //adjusting value
			break;
		}
	}
}


//-------------------------------------------------

bool exists_in_arr(pid_t arr[], pid_t pid, int *filled)
{
  int index;

  //checks in filled part of array if already exists
  for(index = 0; index < *filled; index++)
    if(arr[index] == pid)
      return true;

  return false;
}

//-------------------------------------------------

void perror_and_exit(char *action)
{
	perror(action);
	exit(EXIT_FAILURE);
}
