#include "time.h"
#include "unistd.h"
#include "sys/time.h"
#include "stdlib.h"
#include "stdio.h"

// illustration of accumulating drift

/*
  Tasks:
    - experiment with different computations and their impact on time difference
    - try to run the program with low/high "background" (other processes on the computer)
      does it have impact on the accumulation of drift?
    - create program with two or more threads
       does it have impact on accumulation of drift?
 */

int main()
{

  struct timeval end, start;
  long int delay;
  int sec;
  int us;
  int sum = 0, i, j;

  gettimeofday(&start, NULL);
  printf("Start: time %ld   %ld\n", start.tv_sec, start.tv_usec);

  for (i = 0; i < 8; i++)
  {

    gettimeofday(&end, NULL);

    // core code
    sum = 0;
    for (j = 0; j < 10000; j++)
      sum += j;

    printf("Iter %d\tresult %d\n", i, sum);

    delay = (end.tv_sec * 1000000 + end.tv_usec) -
                     (start.tv_sec * 1000000 + start.tv_usec);
    sec = delay/1000000;
    us = delay%1000000;
    printf("    difference: %d seconds, %d us.\n",sec, us);

    sleep(1);
  }

  return 0;
}
