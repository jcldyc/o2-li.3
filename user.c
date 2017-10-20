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

  if ((id = shmget(shmKey,sizeof(shm), IPC_CREAT | 0666)) < 0){
       perror("SHMGET");
       exit(1);
   }

   if((shmPtr = shmat(id, NULL, 0)) == (shmData *) -1){
       perror("SHMAT");
       exit(1);
   }

   sem_t *sem = sem_open("mySem", O_RDWR);
   if (sem == SEM_FAILED) {
       perror("sem_open(3) failed in child");
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
    sem_wait(sem);      //waits for the semaphore to free up

    if((shmPtr->seconds >= shmSeconds) && (shmPtr->nanoseconds >= shmNano) && shmPtr->shmMsg[1] == 0){
      shmPtr->shmMsg[0] = shmSeconds;
      shmPtr->shmMsg[1] = shmNano;
      criticalFlag = 0;
      sem_post(sem);
    }
    sem_post(sem);
  }

  //printf("\ninside of user process;  pid: %ld ; shmMsg: %s\n", (long)pid, shmMsgContent);
  //printf("\nSeconds and nanoseconds: %d . %d", shmSeconds, shmNano);
  //strncpy(shmPtr->shmMsg, shmMsgContent, 256);
  shmdt(shmPtr);					//must detach the shared memory
  //shmctl(id, IPC_RMID, NULL);		//setting the control of the shared memory using the variable id = shmget(key,sizeof(shm), IPC_CREAT | 0666);
  //kill(pid, SIGTERM);
//  sem_post (sem);
  return 0;

}
