/* producer-consumer with semaphores */
#include "types.h"
#include "user.h"

#undef NULL
#define NULL ((void*)0)

#define PGSIZE (4096)

#define N 10
volatile int bufsize;
int *buffer;
int fillptr, useptr;
int ppid;
//sem_t mutex, full, empty;
cond_t empty;  // condition signal
cond_t full;   // condition signal
lock_t mutex;
#define assert(x) if (x) {} else { \
   printf(1, "%s: %d ", __FILE__, __LINE__); \
   printf(1, "assert failed (%s)\n", # x); \
   printf(1, "TEST FAILED\n"); \
   kill(ppid); \
   exit(); \
}

void produce(void *arg);
void consume(void *arg);

int
main(int argc, char *argv[])
{
  int i;
  ppid = getpid();
  lock_init(&mutex);
  cond_init(&full);
  cond_init(&empty);
  bufsize = 0;
  buffer = malloc(N * sizeof(int));
  assert(clone(produce, NULL) > 0);
  for (i = 0; i < 7; i++)
    assert(clone(consume, NULL) > 0);

  assert(join() > 0);
  sleep(20);
  printf(1,"bufsize = %d\n",bufsize);
  assert(bufsize == 0);
  printf(1, "TEST PASSED\n");


  exit();
}

void fill(int val) {
  buffer[fillptr] = val;
  fillptr = (fillptr + 1) % N;
  bufsize++;
}

int get() {
  int tmp = buffer[useptr];
  useptr = (useptr + 1) % N;
  int t = bufsize - 1;
//  bufsize--;
  sleep(1);
  bufsize = t;
  return tmp;
}

void
produce(void *arg) {
  int i;
  for(i = 0; i < 100; i++) {
   lock_acquire(&mutex);
   while (bufsize == N) {
    cond_wait(&empty, &mutex);
   }
   fill(i);
   cond_signal(&full);
   lock_release(&mutex);
   printf(1, "producer %d : %d\n", getpid(), i);
  }
}

void
consume(void *arg) {
   int i;
   for(i = 0; i < 100; i++) {
     lock_acquire(&mutex);
     while(bufsize == 0){
      cond_wait(&full, &mutex);
     }
     int tmp = get();
     cond_signal(&empty);
     lock_release(&mutex);
     printf(1, "consumer %d : %d\n", getpid(), tmp);
   }
}
   
