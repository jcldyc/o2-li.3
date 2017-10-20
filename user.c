#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <string.h>
#include <semaphore.h>
#include <fcntl.h>

#define billion 1000000000

#define SEM_NAME "/SEMMY"

void exitfuncCtrlC(int sig);

enum message {
 parent, child_terminate
};

typedef struct ShmData{             //struct used to hold the seconds, nanoseconds, and shmMsg and reference in shared memory
  int seconds;
  int nanoseconds;
  long shmMsg[2];

  //enum message shmMsg;
}shmData;

shmData shm;
shmData *shmPtr;

int main (int argc, char *argv[]){
  shmPtr = &shm;
  int id;
  int shmKey = 3699;
  pid_t pid = getpid();
  char shmMsgContent[256];
  FILE *file_ptr;
  long randomTime =0;
  int criticalFlag = 1;

  //printf("check check");

  if (signal(SIGINT, exitfuncCtrlC) == SIG_ERR) {
       printf("SIGINT error\n");
       exit(1);
   }

  if ((id = shmget(shmKey,sizeof(shm), IPC_CREAT | 0666)) < 0){
       perror("SHMGET");
       exit(1);
   }

   if((shmPtr = shmat(id, NULL, 0)) == (shmData *) -1){
       perror("SHMAT");
       exit(1);
   }

   sem_t *semaphore = sem_open(SEM_NAME, O_RDWR);
   if (semaphore == SEM_FAILED) {
     perror("sem_open(3) failed");
     exit(EXIT_FAILURE);
   }



//gett the time from shmMsg, add the random time to it, shmSeconds and
  long shmSeconds = shmPtr->seconds;
  long shmNano = shmPtr->nanoseconds;
  randomTime = rand() % (1000000 + 1 - 0) + 0;
  long newNanoTime = shmNano + randomTime;

  if((newNanoTime % billion) != newNanoTime){
    shmSeconds++;
    shmNano = newNanoTime % billion;
  }


  while(criticalFlag){
    //printf("made into crtical loop");
    if (sem_wait(semaphore) < 0) {
            perror("sem_wait(3) failed on child");
            continue;
    }


    if((shmPtr->seconds >= shmSeconds) && (shmPtr->nanoseconds >= shmNano) && shmPtr->shmMsg[1] == 0){
      printf("inside child, about to terminate.  PID:  %d \n", getpid());
      shmPtr->shmMsg[0] = shmSeconds;
      shmPtr->shmMsg[1] = shmNano;
      criticalFlag = 0;
    }

    if(sem_post(semaphore) < 0) {
      perror("sem_post(3) error on child");
    }
    //sleep(1);

  }
  if (sem_close(semaphore) < 0)
    perror("sem_close(3) failed");
  shmdt(shmPtr);					//must detach the shared memory

  return 0;

}

void exitfuncCtrlC(int sig){

    fprintf( stderr, "Child %ld is dying from parent\n", (long)getpid());
    shmdt(shmPtr);
    sem_unlink(SEM_NAME);
    exit(1);
}
