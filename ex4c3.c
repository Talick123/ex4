

// --------include section------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

// --------const section------------------------

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
void registr(int misqid1, struct Data1 &msg1)
{

}

//-------------------------------------------------

void read_from_user(int misqid1, struct Data1 &msg1, int misqid2, struct Data2 &msg2)
{

}



//-------------------------------------------------
