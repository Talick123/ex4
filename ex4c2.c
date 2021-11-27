//
// void is_palindrome(int arr[], int n)
// {
//     // Initialise flag to zero.
//     int flag = 0;
//
//     // Loop till array size n/2.
//     for (int i = 0; i <= n / 2 && n != 0; i++) {
//
//         // Check if first and last element are different
//         // Then set flag to 1.
//         if (arr[i] != arr[n - i - 1]) {
//             flag = 1;
//             break;
//         }
//     }
//
//     // If flag is set then print Not Palindrome
//     // else print Palindrome.
//     if (flag == 1)
//         cout << "Not Palindrome";
//     else
//         cout << "Palindrome";
// }


// --------include section------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// --------const and enum section------------------

enum Requests {PRIME, PALINDROME};
enum Answers {FALSE, TRUE, FAIL = -1}; //MAYBE JUST CONVERT BOOLS?
enum Exists_answers {DOESNTEXIST, DOESEXIST};
const int REGISTER = 1;

// --------struct section------------------------

struct Data1 {
	pid_t _cpid; //child pid
  int _status;
};

struct Msgbuf1{
  long _type;
  struct Data1 _data1;
};

struct Data2 {
	pid_t _cpid; //child pid
  int _status;
  int _num;
  char* _string;
};

struct Msgbuf2{
  long _type;
  struct Data2 _data2;
};

// --------prototype section---------------------

void catch_int(int signum);
void read_requests(int misqid1, struct Data1 &msg1, int misqid2, struct Data2 &msg2);
int is_prime(int num);
int is_palindrome(char *string);

// --------main section------------------------

int main()
{
  struct Msgbuf1 msg1;
  struct Msgbuf2 msg2;
  int msqid1, msqid2;
  key_t key1, key2;

  signal(SIGINT, catch_int);
  //creating external id for message queue
  if((key2 = ftok(".",'d')) == -1 ||
      (key1 = ftok(".", 'c')) == -1)
  {
    perror("ftok failed");
    exit(EXIT_FAILURE);
  }

  //creating internal id for message queue
  if((msqid2 = msgget(key2, 0600 | IPC_CREAT | IPC_EXCL)) == -1 ||
      ((msqid1 = msgget(key1, 0)) == -1))
  {
    perror("msgget failed");
    exit(EXIT_FAILURE);
  }

  read_requests(msqid1, msg1, msqid2, msg2);

  return EXIT_SUCCESS;
}

//-------------------------------------------------


void catch_int(int signum)
{
  //release queue - can it do it here or does it need to do in main
  //ends
}

//-------------------------------------------------

void read_requests(int misqid1, struct Data1 &msg1, int misqid2, struct Data2 &msg2)
{
  int status;

  while(true)
  {
    if(msgrv(msqid2, &msg2, sizeof(struct Data2), 2, 0) == -1)
    {
      perror("msgrcv failed");
      exit(EXIT_SUCCESS);
    }

    //preparing to ask registry server to check if customer exists
    msg1._type = REGISTER;
    msg1._data1._cpid = msg2._data2._cpid;
    msg1._data1._status = DOESEXIST;
    if(msgsnd(msqid1, &msg1, sizeof(struct Data1), 0) == -1) //sending request
    {
      perror("msgsnd failed");
      exit(EXIT_FAILURE);
    }
    if(msgrv(msqid1, &msg1, sizeof(struct Data2), getpid(), 0) == -1)//receiving answer (maybe sleep a bit before)
    {
      perror("msgrcv failed");
      exit(EXIT_SUCCESS);
    }

    if(msg1._data1._status == DOESNTEXIST) //checking if customer is in registry server
      status = FAIL;

    else //If customer does exist in registry
    {
      switch(msg2._data2._status) //checking request
      {
        case PRIME:
          status = is_prime(msg2._data._num);
          break;
        case PALINDROME:
          status = is_palindrome(msg2._data._string);
          break;
      }
    }

    //preparing to send back to customer the answer
    msg2._type = msg2._data2._cpid;
    msg2._data2._status = status;
    if(msgsnd(msqid2, &msg2, sizeof(struct Data), 0) == -1)
    {
      perror("msgsnd failed");
      exit(EXIT_FAILURE);
    }

  }
}

//-------------------------------------------------

int is_prime(int num)
{

}

//-------------------------------------------------

int is_palindrome(char *string)
{

}



//-------------------------------------------------
