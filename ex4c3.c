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
#include <unistd.h>
#include <string.h>
#include <ctype.h>

#define ARR_SIZE 100

// --------const and enum section-------------------

enum Requests_1 {ADD, INARR, REMOVE};
enum Add_answers {ADDED, EXISTS, FULL};
enum App_answers {FALSE = 0, TRUE = 1, FAIL = -1};
enum Requests_2 {PRIME, PALINDROME};
const int REGISTER = 1;
const int APPLICATION = 2;


// --------struct section------------------------

struct Data1 {
	pid_t _cpid; //child pid
	pid_t _return;
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
  char _string[ARR_SIZE];
};

struct Msgbuf2{ //msg to application server
  long _type;
  struct Data2 _data2;
};

// --------prototype section---------------------

void registr(int misqid1, struct Msgbuf1 *msg1);
void read_from_user(int misqid1, struct Msgbuf1 *msg1, int misqid2, struct Msgbuf2 *msg2);
void is_num_prime(int misqid2, struct Msgbuf2 *msg2, int num);
void is_str_palindrome(int misqid2, struct Msgbuf2 *msg2, char str[]);
void exit_from_system(int misqid1, struct Msgbuf1 *msg1);
void perror_and_exit(char *action);

// --------main section------------------------

int main()
{
	struct Msgbuf1 msg1;
	struct Msgbuf2 msg2;
	int msqid1, msqid2;
	key_t key1, key2;

	msg2._data2._cpid = msg1._data1._cpid =  msg1._data1._return = getpid();

	//creating external id for message queue
	if((key2 = ftok(".",'d')) == -1 ||
		(key1 = ftok(".", 'c')) == -1)
		perror_and_exit("ftok failed");

	//creating internal id for message queue
	if((msqid2 = msgget(key2, 0)) == -1 ||
		((msqid1 = msgget(key1, 0)) == -1))
		perror_and_exit("msgget failed");

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

}

//-------------------------------------------------

void read_from_user(int misqid1, struct Msgbuf1 *msg1, int misqid2, struct Msgbuf2 *msg2)
{
	char req;
	char str[ARR_SIZE];
	int num;

	while (true)
	{
		req = getchar(); //reading char

		switch (req) {
			case 'n': //reading number
				scanf(" %d", &num);
				is_num_prime(misqid2, msg2, num);
				break;
			case 's': //reading string
				scanf(" %s", str);
				is_str_palindrome(misqid2, msg2, str);
				break;
			case 'e': //ending
				exit_from_system(misqid1, msg1);
				break;
			default:
				break;
		}
	}
}

//-------------------------------------------------

void is_num_prime(int misqid2, struct Msgbuf2 *msg2, int num)
{
	//preparing to send to application
	(*msg2)._type = APPLICATION;
	(*msg2)._data2._status = PRIME;
	(*msg2)._data2._num = num;

	//sending request
	if(msgsnd(misqid2, msg2, sizeof(struct Data2), 0) == -1)
		perror_and_exit("msgsnd failed");

	//receiving answer
	if(msgrcv(misqid2, msg2, sizeof(struct Data2), getpid(), 0) == -1)
		perror_and_exit("msgrcv failed");

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

void is_str_palindrome(int misqid2, struct Msgbuf2 *msg2, char str[])
{
	//preparing to send to application
	(*msg2)._type = APPLICATION;
	(*msg2)._data2._status = PALINDROME;
	int index;

	//adding string to message
	for(index = 0; !isspace(str[index]); index++)
		(*msg2)._data2._string[index] = str[index];

	(*msg2)._data2._string[index] = '\0'; //null terminating

	//sending request
	if(msgsnd(misqid2, msg2, sizeof(struct Data2), 0) == -1)
		perror_and_exit("msgsnd failed");

	//receiving answer
	if(msgrcv(misqid2, msg2, sizeof(struct Data2), getpid(), 0) == -1)
		perror_and_exit("msgrcv failed");

	//printing answer
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
	//preparing to send to register
	(*msg1)._type = REGISTER;
	(*msg1)._data1._status = REMOVE;

	//requesting to remove from register
	if(msgsnd(misqid1, msg1, sizeof(struct Data1), 0) == -1)
		perror_and_exit("msgsnd failed");

	printf("I've been removed from the registry. Bye!\n");
	exit(EXIT_SUCCESS);
}

//-------------------------------------------------

void perror_and_exit(char *action)
{
	perror(action);
	exit(EXIT_FAILURE);
}
