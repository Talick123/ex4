/*
 TODO: CHANGE STRING TO STATIC
*/

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

// --------const and enum section-------------------

enum Requests_1 {ADD, INARR, REMOVE}; // add and remvoe
enum Add_answers {ADDED, EXISTS, FULL}; // need just full
enum App_answers {FALSE = 0, TRUE = 1, FAIL = -1};
enum Requests_2 {PRIME, PALINDROME}; // need
const int REGISTER = 1; //can change this to type enum that equals 1 and 2
const int APPLICATION = 2;

// --------struct section------------------------

struct Data1 { //data to registry server
	pid_t _cpid; //child pid
  int _status;
};

struct Msgbuf1{ //msg to registry server
  long _type;
  struct Data1 _data1;
};

struct Data2 { //data to application server
	pid_t _cpid; //child pid
  int _status;
  int _num;
  char* _string;
};

struct Msgbuf2{ //msg to application server
  long _type;
  struct Data2 _data2;
};

// --------prototype section---------------------

void registr(int misqid1, struct Msgbuf1 *msg1);
void read_from_user(int misqid1, struct Msgbuf1 *msg1, int misqid2, struct Msgbuf2 *msg2);
void is_num_prime(int misqid2, struct Msgbuf2 *msg2, int num);
void is_str_palindrome(int misqid2, struct Msgbuf2 *msg2, char *str);
void exit_from_system(int misqid1, struct Msgbuf1 *msg1);
void perror_and_exit(char *action);

// --------main section------------------------

int main()
{
	struct Msgbuf1 msg1;
	struct Msgbuf2 msg2;
	int msqid1, msqid2;
	key_t key1, key2;

	//T: should we put the next 3 things in function??
	msg2._data2._cpid = msg1._data1._cpid =  getpid();

	//creating external id for message queue
	if((key2 = ftok(".",'d')) == -1 ||
		(key1 = ftok(".", 'c')) == -1)
		perror_and_exit("ftok failed");

	//creating internal id for message queue
	if((msqid2 = msgget(key2, 0)) == -1 ||
		((msqid1 = msgget(key1, 0)) == -1))
		perror_and_exit("msgget failed");
	printf("msqid1 is: %d\n",msqid1);
	printf("msqid2 is: %d\n",msqid2);

	registr(msqid1, &msg1);
	read_from_user(msqid1, &msg1, msqid2, &msg2);

	return EXIT_SUCCESS;
}

//-------------------------------------------------

void registr(int misqid1, struct Msgbuf1 *msg1)
{
	(*msg1)._data1._status = ADD;
	(*msg1)._type = REGISTER;

	//send request to register
	if(msgsnd(misqid1, msg1, sizeof(struct Data1), 0) == -1)
		perror_and_exit("msgsnd failed");

	//read result -- can add to this if the check if full?
	if(msgrcv(misqid1, msg1, sizeof(struct Data1), getpid(), 0) == -1)
		perror_and_exit("msgrcv failed");

	//check if succeeded
	if((*msg1)._data1._status == FULL)
		perror_and_exit("registration failed");

	//...else register success . so ok
}

//-------------------------------------------------

void read_from_user(int misqid1, struct Msgbuf1 *msg1, int misqid2, struct Msgbuf2 *msg2)
{
	char req;
	char *str = NULL;
	int num;

// T: yoram said something about spaces???
	while (true)
	{
		req = getchar();
		switch (req) {
			case 'n':
				scanf(" %d", &num);
				is_num_prime(misqid2, msg2, num);
				break;
			case 's':
				scanf(" %s", str);
				is_str_palindrome(misqid2, msg2, str);
				break;
			case 'e':
				exit_from_system(misqid1, msg1);
				break; //T: needed?
			default:
				break;
		}
	}
}

//-------------------------------------------------

void is_num_prime(int misqid2, struct Msgbuf2 *msg2, int num)
{
	(*msg2)._type = APPLICATION;
	(*msg2)._data2._status = PRIME;
	(*msg2)._data2._num = num;

	if(msgsnd(misqid2, msg2, sizeof(struct Data2), 0) == -1)
		perror_and_exit("msgsnd failed");

	if(msgrcv(misqid2, msg2, sizeof(struct Data2), getpid(), 0) == -1)
		perror_and_exit("msgrcv failed");

	//we can do it in one printf
	switch ((*msg2)._data2._status)
	{
		case FALSE:
			printf("%d is not a prime\n", num);
			break;
		case TRUE:
			printf("%d is a prime\n", num);
			break;
		case FAIL:
			printf("I'm not registered\n");
	}
}

//-------------------------------------------------

void is_str_palindrome(int misqid2, struct Msgbuf2 *msg2, char *str)
{
	(*msg2)._type = APPLICATION;
	(*msg2)._data2._status = PALINDROME;
	(*msg2)._data2._string = str;

	if(msgsnd(misqid2, msg2, sizeof(struct Data2), 0) == -1)
		perror_and_exit("msgsnd failed");

	if(msgrcv(misqid2, msg2, sizeof(struct Data2), getpid(), 0) == -1)
		perror_and_exit("msgrcv failed");

	//we can do it in one printf
	switch ((*msg2)._data2._status) {
		case FALSE:
			printf("%s is not a palindrome\n", str);
			break;
		case TRUE:
			printf("%s is a palindrome\n", str);
			break;
	}
}

//-------------------------------------------------

// ?
void exit_from_system(int misqid1, struct Msgbuf1 *msg1)
{
	(*msg1)._type = REGISTER;
	(*msg1)._data1._status = REMOVE;

	if(msgsnd(misqid1, msg1, sizeof(struct Data1), 0) == -1)
		perror_and_exit("msgsnd failed");

	printf("bye\n");
	exit(EXIT_SUCCESS);
}

//-------------------------------------------------

void perror_and_exit(char *action)
{
	perror(action);
	exit(EXIT_FAILURE);
}
