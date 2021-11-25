


// --------include section------------------------

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h> //for pipe
#include <sys/wait.h>
#include <sys/time.h>


int main()
{
  //receives via argument vector the number to send to aba
  //sends to aba starting number
  //waits for number back in order to start
  //randomizes numbers in an infinite loop
  //each time there is prime, sends to first program through named pipe
  //when ending message received, prints how many new primes sent
  //ends
  return EXIT_SUCCESS;
}
