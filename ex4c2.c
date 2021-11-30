// int solve(char *string){
//    int length;
//    char *forward, *reverse;
//    length = strlen(string);
//    forward = string;
//    reverse = forward + length - 1;
//    for (forward = string; reverse >= forward;) {
//       if (*reverse == *forward) {
//          reverse--;
//          forward++;
//       } else
//          break;
//    } if (forward > reverse)
//       return 1;
//    else
//       return 0;
// }


// --------include section------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

// --------const and enum section------------------

enum Requests {PRIME, PALINDROME};
enum Answers {FALSE = 0, TRUE = 1, FAIL = -1}; //MAYBE JUST CONVERT BOOLS?
enum Exists_answers {DOESNTEXIST, DOESEXIST};
const int REGISTER = 1;
int msqid2;

// --------struct section------------------------

struct Data1 {
	pid_t _cpid; //child pid
	pid_t _return;
	int _status;
};

struct Msgbuf1{ //to registry
  long _type;
  struct Data1 _data1;
};

struct Data2 { //to customer
	pid_t _cpid; //child pid
  int _status;
  int _num;
  char _string[100];
};

struct Msgbuf2{ //to customer
  long _type;
  struct Data2 _data2;
};

// --------prototype section---------------------

void catch_int(int signum);
void read_requests(int msqid1, struct Msgbuf1 *msg1, int msqid2, struct Msgbuf2 *msg2);
int is_prime(int num);
int is_palindrome(char *string);

// --------main section------------------------

int main()
{
  struct Msgbuf1 msg1;
  struct Msgbuf2 msg2;
  int msqid1;
  key_t key1, key2;

  signal(SIGINT, catch_int);

  //creating external id for message queue
  if((key2 = ftok(".",'d')) == -1 ||
      (key1 = ftok(".", 'c')) == -1)
  {
    perror("ftok failed\n");
    exit(EXIT_FAILURE);
  }

  //creating internal id for message queue
  if(((msqid2 = msgget(key2, 0600 | IPC_CREAT | IPC_EXCL)) == -1 && errno != EEXIST)||
      ((msqid1 = msgget(key1, 0)) == -1))
  {
    perror("msgget failed\n");
    exit(EXIT_FAILURE);
  }
  printf("msqid1 is: %d\n",msqid1);
  printf("msqid2 is: %d\n",msqid2);

  read_requests(msqid1, &msg1, msqid2, &msg2);

  return EXIT_SUCCESS;
}

//-------------------------------------------------

void catch_int(int signum)
{

	if(msgctl(msqid2, IPC_RMID, NULL) == -1)
	{
		perror("msgctl faild\n");
		exit(EXIT_FAILURE);
	}
	exit(EXIT_SUCCESS);
}

//-------------------------------------------------

void read_requests(int msqid1, struct Msgbuf1 *msg1, int msqid2, struct Msgbuf2 *msg2)
{
  int status;

  while(true)
  {
	printf("about to receive request\n");
    if(msgrcv(msqid2, msg2, sizeof(struct Data2), 2, 0) == -1)
    {
      perror("msgrcv failed\n");
      exit(EXIT_SUCCESS);
    }

    printf("received request, asking registry\n");

    //preparing to ask registry server to check if customer exists
    (*msg1)._type = REGISTER;
    (*msg1)._data1._cpid = (*msg2)._data2._cpid;
    (*msg1)._data1._return = getpid();
    (*msg1)._data1._status = DOESEXIST;
    if(msgsnd(msqid1, msg1, sizeof(struct Data1), 0) == -1) //sending request
    {
      perror("msgsnd failed\n");
      exit(EXIT_FAILURE);
    }

    printf("sent to registry\n");

    if(msgrcv(msqid1, msg1, sizeof(struct Data1), getpid(), 0) == -1)//receiving answer (maybe sleep a bit before)
    {
      perror("msgrcv failed\n");
      exit(EXIT_SUCCESS);
    }

    if((*msg1)._data1._status == DOESNTEXIST) //checking if customer is in registry server
      status = FAIL;

    else //If customer does exist in registry
    {
      switch((*msg2)._data2._status) //checking request
      {
        case PRIME:
          status = is_prime((*msg2)._data2._num);
          break;
        case PALINDROME:
          status = is_palindrome((*msg2)._data2._string);
          break;
      }
    }

    //preparing to send back to customer the answer
    (*msg2)._type = (*msg2)._data2._cpid;
    (*msg2)._data2._status = status;
    if(msgsnd(msqid2, msg2, sizeof(struct Data2), 0) == -1)
    {
      perror("msgsnd failed\n");
      exit(EXIT_FAILURE);
    }
  }
}

//-------------------------------------------------

int is_prime(int num)
{
	int i;
	for(i = 2; i*i <= num; i++)
	{
		if(num % i == 0)
			return FALSE;//change
	}
	return TRUE;//change
}

//-------------------------------------------------

int is_palindrome(char *string)
{
	printf("%s\n", string);

	int length;
	char *forward, *reverse;
	length = strlen(string);
	forward = string;
	reverse = forward + length - 1;
	for (forward = string; reverse >= forward;)
	{
		if (*reverse == *forward)
		{
			reverse--;
			forward++;
		}
		else
		 break;
	}
	if (forward > reverse)
		return TRUE;
	else
		return FALSE;
}

//-------------------------------------------------
