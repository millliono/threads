#include <pthread.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/time.h>

#define QUEUESIZE 10
#define LOOP 20
#define NUM_CONSUMER 5
#define PERIOD 10000

void *producer(void *args);
void *consumer(void *args);

typedef struct
{
  int buf[QUEUESIZE];
  long head, tail;
  int full, empty;
  pthread_mutex_t *mut;
  pthread_cond_t *notFull, *notEmpty;
} queue;

queue *queueInit(void);
void queueDelete(queue *q);
void queueAdd(queue *q, int in);
void queueDel(queue *q, int *out);

bool prod_finished = 0;
struct timeval tv;
time_t t;
struct tm *info;

void dowork()
{
  printf("Thread is running.\n");
  for (int i = 0; i < 1000000; ++i)
  {
    // Do some computation
  }
  printf("Thread is done.\n");
}

int main()
{
  queue *fifo;
  pthread_t pro;
  pthread_t cons[NUM_CONSUMER];

  fifo = queueInit();
  if (fifo == NULL)
  {
    fprintf(stderr, "main: Queue Init failed.\n");
    exit(1);
  }

  pthread_create(&pro, NULL, producer, fifo);
  for (int t = 0; t < NUM_CONSUMER; t++)
  {
    pthread_create(&cons[t], NULL, consumer, fifo);
  }

  pthread_join(pro, NULL);
  for (int t = 0; t < NUM_CONSUMER; t++)
  {
    pthread_join(cons[t], NULL);
  }

  queueDelete(fifo);

  return 0;
}

void *producer(void *q)
{
  queue *fifo;
  int i;

  fifo = (queue *)q;

  struct timeval end;
  struct timeval start;
  unsigned int delay;

  for (i = 0; i < LOOP; i++)
  {
    gettimeofday(&start, NULL);
    pthread_mutex_lock(fifo->mut); // lock mutex

    while (fifo->full)
    {
      printf("producer: queue FULL.\n");
      pthread_cond_wait(fifo->notFull, fifo->mut);
    }
    gettimeofday(&end, NULL);
    queueAdd(fifo, i); // add

    gettimeofday(&tv, NULL);
    t = tv.tv_sec;
    info = localtime(&t);
    printf("%s", asctime(info));

    pthread_mutex_unlock(fifo->mut); // unlock mutex
    pthread_cond_signal(fifo->notEmpty);

    delay = (end.tv_sec * 1000000 + end.tv_usec) -
            (start.tv_sec * 1000000 + start.tv_usec);
    if ((PERIOD - delay) > 0)
    {
      usleep(PERIOD - delay);
    }
  }
  prod_finished = 1;
  pthread_cond_broadcast(fifo->notEmpty);
  return (NULL);
}

void *consumer(void *q)
{
  queue *fifo;
  int i, d;

  fifo = (queue *)q;

  while (1)
  {
    pthread_mutex_lock(fifo->mut);
    while (fifo->empty && !prod_finished)
    {
      printf("consumer: queue EMPTY.\n");
      pthread_cond_wait(fifo->notEmpty, fifo->mut);
    }
    if (fifo->empty && prod_finished)
    {
      pthread_mutex_unlock(fifo->mut);
      break;
    }
    queueDel(fifo, &d);
    pthread_mutex_unlock(fifo->mut);
    pthread_cond_signal(fifo->notFull);
    printf("consumer: recieved %d.\n", d);
  }
  return (NULL);
}

queue *queueInit(void)
{
  queue *q;

  q = (queue *)malloc(sizeof(queue));
  if (q == NULL)
    return (NULL);

  q->empty = 1;
  q->full = 0;
  q->head = 0;
  q->tail = 0;
  q->mut = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  pthread_mutex_init(q->mut, NULL);
  q->notFull = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
  pthread_cond_init(q->notFull, NULL);
  q->notEmpty = (pthread_cond_t *)malloc(sizeof(pthread_cond_t));
  pthread_cond_init(q->notEmpty, NULL);

  return (q);
}

void queueDelete(queue *q)
{
  pthread_mutex_destroy(q->mut);
  free(q->mut);
  pthread_cond_destroy(q->notFull);
  free(q->notFull);
  pthread_cond_destroy(q->notEmpty);
  free(q->notEmpty);
  free(q);
}

void queueAdd(queue *q, int in)
{
  q->buf[q->tail] = in;
  q->tail++;
  if (q->tail == QUEUESIZE)
    q->tail = 0;
  if (q->tail == q->head)
    q->full = 1;
  q->empty = 0;

  return;
}

void queueDel(queue *q, int *out)
{
  *out = q->buf[q->head];

  q->head++;
  if (q->head == QUEUESIZE)
    q->head = 0;
  if (q->head == q->tail)
    q->empty = 1;
  q->full = 0;

  return;
}
