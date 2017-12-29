#include "server_impl.h"
#include "request.h"
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdlib.h>

int i, numfull, produce, consume, numbuf, numtrd;
int *buffer;

pthread_cond_t empty;  // condition signal
pthread_cond_t fill;   // condition signal
pthread_mutex_t mutex;  // mutex
pthread_t *t;
void* consumer();

void server_init(int argc, char* argv[]) {
  if (argc != 4) {
    fprintf(stderr, "Usage: %s server"
    " [portnum] [threads] [buffers]\n", argv[0]);
    exit(1);
  }
  if (atoi(argv[2]) <= 0) {
    perror("number of threads not positive");
    exit(1);
  }
  if (atoi(argv[3]) <= 0) {
    perror("number of buffers not positive");
    exit(1);
  }

// create pool of threads here
  numbuf = atoi(argv[3]);
  buffer = malloc(sizeof(uint)*numbuf);
// may need to create a struct in ulib.h
  for (i = 0; i < numbuf; i++) buffer[i] = -1;

// -1 means free
  numfull = 0;
  produce = 0;
  consume = 0;

  numtrd = atoi(argv[2]);
  t = (pthread_t *)malloc(sizeof(pthread_t)*numtrd);
  if (pthread_cond_init(&empty, NULL) != 0) exit(1);
  if (pthread_cond_init(&fill, NULL) != 0) exit(1);
  if (pthread_mutex_init(&mutex, NULL) != 0) exit(1);


  for (i = 0; i < numtrd; i++) {
    if (pthread_create(&t[i], NULL, consumer, NULL) != 0) {
      printf("pthread_create goes wrong\n");
      exit(1);
    }
  }
}
void *consumer() {
// a consumer, when locking fill a buffer, when unlock, free a buffer
  int fd1 = 0;
  while (1) {
    pthread_mutex_lock(&mutex);

    while (numfull == 0) {
      pthread_cond_wait(&fill, &mutex);
    }
    fd1 = buffer[consume];
    buffer[consume] = -1;
    consume = (consume+1)%numbuf;
    numfull -= 1;

    pthread_cond_signal(&empty);
    pthread_mutex_unlock(&mutex);
    requestHandle(fd1);
  }
}

void server_dispatch(int connfd) {
// Handle the request in the main thread.

  int fd = connfd;

  pthread_mutex_lock(&mutex);
  while (numfull == numbuf) {
    pthread_cond_wait(&empty, &mutex);
  }

  buffer[produce] = fd;
  produce = (produce+1)%numbuf;
  numfull += 1;
  pthread_cond_signal(&fill);
  pthread_mutex_unlock(&mutex);
}
