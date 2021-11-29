
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
int in_arr(pid_t arr[], pid_t pid, int filled);
void remove_from_arr(pid_t arr[], pid_t pid, int *filled);
bool exists_in_arr(pid_t arr[], pid_t pid, int filled);

// --------main section--------------------------

int main()
{
	struct Msgbuf msg;
	key_t key;

	signal(SIGINT, catch_int);

	//creating external id for message queue
	if((key = ftok(".",'c')) == -1)
	{
		perror("ftok failed\n");
		exit(EXIT_FAILURE);
	}

	//creating internal id for message queue
	if((msqid = msgget(key, 0600 | IPC_CREAT | IPC_EXCL)) == -1 && errno != EEXIST)
	{
		perror("msgget failed\n");
		exit(EXIT_FAILURE);
	}
	printf("msqid is: %d\n",msqid);
	read_requests(msqid, &msg);

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void catch_int(int signum)
{
	if(msgctl(msqid, IPC_RMID, NULL) == -1)
	{
		perror("msgctl failed\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

//-------------------------------------------------

void read_requests(int msqid, struct Msgbuf *msg)
{
  pid_t arr[ARR_SIZE];
  int status, filled = 0;

  while(true)
  {
    if(msgrcv(msqid, msg, sizeof(struct Data), 1, 0) == -1)
    {
      perror("msgrcv failed\n");
      exit(EXIT_SUCCESS);
    }

    switch((*msg)._data._status)
    {
	  case ADD:
		//checks if exist in array, or if full,  adds if it can
		status = add_to_arr(arr, (*msg)._data._cpid, &filled);
		break;
	  case INARR:
		//checks if it exists in array
		status = in_arr(arr, (*msg)._data._cpid, filled);
		break;
	  case REMOVE:
		//removes from array
		remove_from_arr(arr, (*msg)._data._cpid, &filled);
		break;
    }

	if((*msg)._data._status != REMOVE)
	{
		(*msg)._type = (*msg)._data._cpid; //prepares to send status back to sender
		(*msg)._data._status = status;

		//sends status to sender
		if(msgsnd(msqid, msg, sizeof(struct Data), 0) == -1)
		{
		  perror("msgsnd failed\n");
		  exit(EXIT_FAILURE);
		}
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
  if(exists_in_arr(arr, pid, *filled))
      return EXISTS;

  //if does not exist, adds and returns
  arr[++(*filled)] = pid;
  return ADDED;

}

//-------------------------------------------------

int in_arr(pid_t arr[], pid_t pid, int filled)
{
  if(exists_in_arr(arr, pid, filled))
    return DOESEXIST;

  return DOESNTEXIST;
}

//-------------------------------------------------

void remove_from_arr(pid_t arr[], pid_t pid, int *filled)
{
	int index, place;

	for(index = 0; index < *filled; index++)
	{
		if(arr[index] == pid)
		{
			for(place = index; place < (*filled) - 1; place++)
				arr[place] = arr[place + 1];

			arr[place] = 0;
			--(*filled);
			break;
		}
	}
}


//-------------------------------------------------

bool exists_in_arr(pid_t arr[], pid_t pid, int filled)
{
  int index;

  //checks in filled part of array if already exists
  for(index = 0; index < filled; index++)
    if(arr[index] == pid)
      return true;

  return false;
}
