#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/time.h>
#include <math.h>
#define SHM_NAME  "ysoo_conradc"
// TODO(student): Include necessary headers
sem_t * mutex;
int fd_shm = -1;
char* addr;
int empty = -1;
typedef struct {
  int pid;
  char birth[25];
  int elapsed_sec;
  double elapsed_msec;
} stats_t;

stats_t *p;
void exit_handler(int sig) {
    // new routine defined here specified by sigaction() in main
    // TODO(student): critical section begins

    sem_wait(mutex);
    // client reset its segment, or mark its segment as valid
    // so that the segment can be used by another client later.
    p->pid = -1;
    sem_post(mutex);
    // critical section ends
    if (munmap(addr, 4096) == -1) {
      perror("munmap add_r failed\n");
      exit(1);
    }

    exit(0);
}

int main(int argc, char *argv[]) {
    // TODO(student): Signal handling
    // Use sigaction() here and call exit_handler
    struct timeval crt, brn;
    struct sigaction act;
    memset (&act, 0, sizeof(act));
    act.sa_handler = exit_handler;
    if (sigaction(SIGINT, &act, NULL) == -1) {
      exit(1);
    }
    if (sigaction(SIGTERM, &act, NULL) == -1) {
      exit(1);
    }
    // TODO(student): Open the preexisting shared memory segment created by
    // shm_server
    fd_shm = shm_open(SHM_NAME, O_RDWR, 0660);
    if (fd_shm == -1) {
      perror("shm failed.\n");
      exit(1);
    }
    addr = (char*)mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (*(int*)addr == -1) {
      perror("mmap add_r failed: ");
      exit(1);
    }
    // TODO(student): point the mutex to the particular segment of the shared
    // memory page
    mutex = (sem_t*)addr;

    stats_t *procs = (stats_t*)(addr+64);


    // TODO(student): critical section begins
    sem_wait(mutex);

    for (int i = 0; i < 63; i++) {
      if (procs[i].pid == -1) {
        empty = i;
        break;
      }
    }
    if (empty != -1) {
      time_t birthtime;
      if (time(&birthtime) == ((time_t)-1)) {
        if (munmap(addr, 4096) == -1) {
          perror("munmap add_r failed\n");
          exit(1);
        }


        exit(1);  ////////////////////////////
      }
      gettimeofday(&brn, NULL);
      char* timestr = ctime(&birthtime);
      if (timestr == NULL) {
        if (munmap(addr, 4096) == -1) {
          perror("munmap add_r failed\n");
          exit(1);
        }


        exit(1);   /////////////////////////////
      }
      strncpy(procs[empty].birth, timestr, strlen(timestr)-1);
      procs[empty].pid = getpid();
      p = procs + empty;
    } else {
      sem_post(mutex);
      if (munmap(addr, 4096) == -1) {
        perror("munmap add_r failed\n");
        exit(1);
      }

      if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlinked failed\n");
        exit(1);
      }
      exit(1);  ///////////////////////////////
    }
    sem_post(mutex);
    // critical section ends

    while (1) {
        // TODO(student): fill in fields in stats_t
//        sem_wait(mutex);
        gettimeofday(&crt, NULL);
        procs[empty].elapsed_sec = crt.tv_sec - brn.tv_sec;
        procs[empty].elapsed_msec = (crt.tv_usec - brn.tv_usec)/1000.0f;
//        sem_post(mutex);
        sleep(1);
        // display the PIDs of all the active clients
        printf("Active clients :");
        for (int i = 0; i < 63; i++) {
          if (procs[i].pid != -1) printf(" %d", procs[i].pid);
        }
        printf("\n");
    }
    return 0;
}

