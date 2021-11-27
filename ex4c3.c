

// --------include section------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// --------const section------------------------

enum Requests_1 {ADD, INARR, REMOVE}; // add and remvoe
enum Add_answers {ADDED, EXISTS, FULL}; // need just full
enum Exists_answers {DOESNTEXIST, DOESEXIST}; // no
enum Requests_2 {PRIME, PALINDROME}; // need
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

void registr(int misqid1, struct Data1 &msg1);
void read_from_user(int misqid1, struct Data1 &msg1, int misqid2, struct Data2 &msg2);

// --------main section------------------------

int main()
{
  struct Msgbuf1 msg1;
  struct Msgbuf2 msg2;
  int msqid1, msqid2;
  key_t key1, key2;

	msg1._data1._cpid =  getpid();

  signal(SIGINT, catch_int);
  //creating external id for message queue
  if((key2 = ftok(".",'d')) == -1 ||
      (key1 = ftok(".", 'c')) == -1)
  {
    perror("ftok failed");
    exit(EXIT_FAILURE);
  }

  //creating internal id for message queue
  if((msqid2 = msgget(key2, 0)) == -1 ||
      ((msqid1 = msgget(key1, 0)) == -1))
  {
    perror("msgget failed");
    exit(EXIT_FAILURE);
  }

  registr(msqid1, msg1);
  read_from_user(msqid1, msg1, msqid2, msg2);

  return EXIT_SUCCESS;
}

//-------------------------------------------------
//maybe not ?
void registr(int misqid1, struct Data1 &msg1)
{
	//msg1._type ?
	msg1._data1._status = ADD;

	//send request to register
	if(msgsnd(misqid1, &msg1, sizeof(struct Data), 0) == -1)
	{
		perror("msgsnd failed");
		exit(EXIT_FAILURE);
	}

	//read result -- can add to this if the check if full?
	if(msgrcv(misqid1, &msg1, sizeof(struct Data), 1, 0) == -1)
	{
		perror("msgrcv failed");
		exit(EXIT_SUCCESS);
	}

	//check if succeeded
	if(msg1._data1._status == FULL)
	{
		perror("array is full");
		exit(EXIT_SUCCESS);
	}
	//...else register success . so ok
}

//-------------------------------------------------

void read_from_user(int misqid1, struct Data1 &msg1, int misqid2, struct Data2 &msg2)
{
	bool end = false;
	char req;
	char *str = NULL;
	int num;

	while (!end) //true?
	{
		req = getchar();
		switch (req) {
			case 'n':
				scanf(" %d", &num);
				is_num_prime(misqid2, msg2, num);
				break;
			case 's':
				scanf(" %s", str);
				is_palindrome(misqid2, msg2, str);
				break;
			case 'e':
				exit_from_system(misqid1, msg1);
				break;
			default:
				break;
		}
	}
}

//-------------------------------------------------

void is_num_prime(int misqid2, struct Data1 &msg2, int num)
{
	msg2._data2._status = PRIME;
	msg2._data2._num = num;
	if(msgsnd(misqid2, &msg2, sizeof(struct Data), 0) == -1)
	{
		perror("msgsnd failed");
		exit(EXIT_FAILURE);
	}

	if(msgrcv(misqid2, &msg2, sizeof(struct Data), 1, 0) == -1)
	{
		perror("msgrcv failed");
		exit(EXIT_SUCCESS);
	}

	//we can do it in one printf
	switch (msg2._data2._status) {
		case FASLE:
			printf("%s is not prime", num);
			break;
		case TRUE:
			printf("%s is not prime", num);
			break;
	}
}

//-------------------------------------------------

void is_str_palindrome(int misqid2, struct Data1 &msg2, char *str)
{
	msg2._data2._status = PALINDROME;
	msg2._data2._string = str;

	if(msgsnd(misqid2, &msg2, sizeof(struct Data), 0) == -1)
	{
		perror("msgsnd failed");
		exit(EXIT_FAILURE);
	}

	if(msgrcv(misqid2, &msg2, sizeof(struct Data), 1, 0) == -1)
	{
		perror("msgrcv failed");
		exit(EXIT_SUCCESS);
	}

	//we can do it in one printf
	switch (msg2._data2._status) {
		case FASLE:
			printf("%s is not palindrome", str);
			break;
		case TRUE:
			printf("%s is not palindrome", str);
			break;
	}
}

//-------------------------------------------------
// ?
void exit_from_system(int misqid1, struct Data1 &msg1)
{
	msg1._data1._status = REMOVE;
	// type?
	if(msgsnd(misqid1, &msg1, sizeof(struct Data), 0) == -1)
	{
		perror("msgsnd failed");
		exit(EXIT_FAILURE);
	}

	if(msgrcv(misqid2, &msg2, sizeof(struct Data), 1, 0) == -1)
	{
		perror("msgrcv failed");
		exit(EXIT_SUCCESS);
	}
	printf("bye\n");
	exit(EXIT_SUCCESS);
}
//-------------------------------------------------
