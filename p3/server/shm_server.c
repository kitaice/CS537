#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/shm.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#define SHM_NAME  "ysoo_conradc"
// TODO(student): Include necessary headers
typedef struct {
  int pid;
  char birth[25];
  int elapsed_sec;
  double elapsed_msec;
} stats_t;
sem_t *mutex;
int ret = -1;
int fd_shm = -1;

char* addr;
int sem = 0;
void exit_handler(int sig) {
    // TODO(student): Clear the shared memory segment
//    ret = munmap(addr, 4096);
    if (munmap(addr, 4096) == -1) {
      perror("munmap add_r failed\n");
      exit(1);
    }

//    shm_unlink(SHM_NAME);
    if (shm_unlink(SHM_NAME) == -1) {
      perror("shm_unlinked failed\n");
      exit(1);
    }
    exit(0);
}
int main(int argc, char **argv) {
    // TODO(student): Signal handling
    // Use sigaction() here and call exit_handler
    struct sigaction act;
    memset (&act, 0, sizeof(act));
    act.sa_handler = exit_handler;
    if (sigaction(SIGINT, &act, NULL) == -1) exit(1);
    if (sigaction(SIGTERM, &act, NULL) == -1) exit(1);
    // TODO(student): Create a new shared memory segment
    fd_shm = shm_open(SHM_NAME, O_RDWR | O_CREAT, 0660);
    if (fd_shm == -1) {
      perror("shm failed.\n");
      exit(1);
    }
    ret = ftruncate(fd_shm, 4096);
    if (ret == -1) {
      perror("truncate failed\n");
      if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlinked failed\n");
        exit(1);
      }
      exit(1);
    }
    addr = (char*)mmap(0, 4096, PROT_READ|PROT_WRITE, MAP_SHARED, fd_shm, 0);
    if (*(int*)addr == -1) {
      perror("mmap add_r failed: ");  ///////     unlink
      if (shm_unlink(SHM_NAME) == -1) {
        perror("shm_unlinked failed\n");
        exit(1);
      }
      exit(1);
    }
    // TODO(student): Point the mutex to the segment of the shared memory page
    // that you decide to store the semaphore
    mutex = (sem_t*)addr;
    sem = sem_init(mutex, 1, 1);  // Initialize semaphore

    // TODO(student): Some initialization of the rest of the segments in the
    // shared memory page here if necessary
    stats_t *procs = (stats_t*)(addr+64);

    for (int i=0; i < 63; i++) {
//      procs[i] = start + i;
      procs[i].pid = -1;
    }
    while (1) {
        // TODO(student): Display the statistics of active clients, i.e. valid
        // segments after some formatting
      sleep(1);
      for (int i = 0; i < 63; i++) {
        if (procs[i].pid != -1) {
          printf("pid : %d, birth : %s, elapsed : %d s %f ms\n",
            procs[i].pid , procs[i].birth, procs[i].elapsed_sec,
            procs[i].elapsed_msec);
        }
     }
    }

    return 0;
}
