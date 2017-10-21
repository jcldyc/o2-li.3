//#define _POSIX_C_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>
#include <sys/shm.h>
#include <string.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <semaphore.h>
#include <errno.h>
#include <time.h>

#define billion 1000000000

// semaphore globals
#define SEM_NAME "/sem"
#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1
#define CHILD_PROGRAM "./user"

void ChildProcess(void);
void ctrlPlusC(int sig);



typedef struct ShmData{             //struct used to hold the seconds, nanoseconds, and shmMsg and reference in shared memory
  long seconds;
  long nanoseconds;
  long shmMsg[2];
  //enum message shmMsg;
}shmData;

shmData shm;
shmData *shmPtr;






int main (int argc, char *argv[]){
  int option;
  int terminateTime =0;
  int maxNumOfSlaves = 5;             //5 is the default amount
  char logFile[] = "logFile";         //set the name of the default log file
  int custLF = 0;                     //flag to determine if user wants a custom named log file; else logFile
  int runIt = 1;                      //flag to determine if program should run.
  FILE* file_ptr;
  pid_t pid;
  int shmKey = 3699;			                //this  is the key used to identify the shared memory

  shmPtr = &shm;			                 //points the shared memory pointer to teh address in shared memory
  int id;
  long loopAdd = 20000000;                 //the amount added to nanoseconds on each loop
  double timeAlotted = 2.0;
  clock_t start_t = clock();




//----------------------getOpt-------------------------------------------------------------------
    while((option=getopt(argc, argv, "hs:l:t:i:t:")) != -1){
      switch(option){
        case 'h':               //this option shows all the arguments available
          runIt = 0;            //how to only asked.  Set to 0 *report to log file*
          printf(" \t./oss: \n\t\t[-s (Max Number of Slaves)] \n\t\t[-l (Log File Name)]\n\t\t[-t (Time to Terminate)] \n\t\t [-i (Amount of nanoseconds added each loop)]\n");
          break;
        case 's':               //allows the user to set the max number of slave processes
          maxNumOfSlaves = atof(optarg);
          printf("\tMax number of processes spawned: %d \n", maxNumOfSlaves);
          break;
        case 'l':               //option to name the logfile used
          custLF = 1;

          if(custLF){
            file_ptr = fopen(optarg, "w");
            fclose(file_ptr);
            file_ptr = fopen(optarg, "a");
            printf("\tLog File used: %s\n", optarg);
          }
          break;
        case 'i':
          loopAdd = atof(optarg);
          printf("\tLoop add time: %ld. \n", loopAdd);
          break;
        case 't':
          timeAlotted = atof(optarg);
          printf("\tTime alotted: %f\nf", timeAlotted);
        default:
          printf("\tno option used \n");
          break;
      }
    }

    //set a print out to let user know default arguments are being used
    if(argc < 3 && runIt){
      printf("\tDefault arguments used. \n");
    }

  //create the log file specified by user
  //otherwise, use the default log file name : logFile

  if(!custLF && runIt){
    file_ptr = fopen(logFile, "w");
    fclose(file_ptr);
    file_ptr = fopen(logFile, "a");
    printf("\tLog File name: %s\n", logFile);
  }
  printf("-----------------------------------------------------------------------\n\n");

//----------------------------------------------------------------------------------------------------



    if (signal(SIGINT, ctrlPlusC) == SIG_ERR) {
        printf("SIGINT error\n");
        exit(1);
    }



    int needMoreProc = 1;
    int processCount = 0;
    int totProcCount = 1; //this will hold the total number of processes ran.  While <100, good to go

    if(runIt){          //if loop keeps the main function of this project to run if the user just wants to know what arguments to use (-h)

      //only creates the shared memory if the program runs through for(runIt)
      id = shmget(shmKey,sizeof(shm), IPC_CREAT | 0666);
      if (id < 0){
        perror("SHMGET");
        exit(1);
      }

      shmPtr = shmat(id, NULL, 0);
      if(shmPtr == (shmData *) -1){
        perror("SHMAT");
        exit(1);
      }


      pid_t cpid[maxNumOfSlaves];             //this array of pid_t holds the pid's of all the processes running currently
                                              //when a child is terminated && totProcCount <100 && time!=totatlAlottedTime, create new child, add id to array

      shmPtr->seconds = 0;
      shmPtr->nanoseconds = 0;
      shmPtr->shmMsg[0] = 0;
      shmPtr->shmMsg[1] = 0;
      //strncpy(shmPtr->shmMsg,shmMsgNull,256);



       /* We initialize the semaphore counter to 1 (INITIAL_VALUE) */
       sem_t *semaphore = sem_open(SEM_NAME, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);

       if (semaphore == SEM_FAILED) {
           perror("sem_open(3) error");
           exit(EXIT_FAILURE);
       }




          /* Close the semaphore as we won't be using it in the parent process */
       if (sem_close(semaphore) < 0) {
           perror("sem_close(3) failed");
           /* We ignore possible sem_unlink(3) errors here */
           sem_unlink(SEM_NAME);
           exit(EXIT_FAILURE);
       }



      // -------------------------Create x amount of slaves --------------------------------------

      while(needMoreProc){
        if(processCount < maxNumOfSlaves){
        //  fprintf(file_ptr, "total process count: %d \n", totProcCount);
          totProcCount++;
          processCount++;
          if ((cpid[processCount] = fork()) == 0){
            sleep(1);
            ChildProcess();
          }
          else if(cpid[processCount]<0){
            printf("\tError forking child");
            processCount--;       //if the process fails, try to fork again and fill that same array space
            totProcCount--;

          }
        }else if (maxNumOfSlaves>=processCount){
          needMoreProc=0;
        }
      }
    //   printf("procCount: %d\n", processCount);
    //   printf("totProcCount: %d\n", totProcCount);
    //
    // printf("-----------------------------------------------------------------------\n\n");
      //----------------------------------------------------------------------------------------------------




      //--------------------------------Loop: check for shmMsg and create new processes---------------------

      int runMainLoop = 1;

      while(runMainLoop){
        //printf("process Count: %d\n", processCount);
        //printf("total process Count: %d\n", totProcCount);


        long sec = shmPtr->seconds;
        long nans = shmPtr->nanoseconds;
        long shmMsgSecond = shmPtr->shmMsg[0];
        long shmMsgNano = shmPtr->shmMsg[1];
        int newForkFlag = 1;
        int working = 0;
        // for(int i=0; i< 10000000; i++) {
        // }
        clock_t end_t = clock();
        clock_t total_t;
        total_t = (double)(end_t - start_t) / CLOCKS_PER_SEC;

        //printf("current time: %f\n", (double)total_t);

        double currTime = (double)total_t;


        int stat;
        pid_t pid1;
      //  pid1 = wait(&stat);
          if(shmPtr->nanoseconds!=0){
            pid1 = wait(&stat);
            //printf("pid1: %d\n", pid1);
            for(int i=0;i<=maxNumOfSlaves;i++){
              if(cpid[i]==pid1){
                fprintf(file_ptr, "Master: Child %ld is terminating at my time %ld.%ld because it reached %ld.%ld in slave \n", (long)pid1, sec, nans, shmMsgSecond,shmMsgNano);
                shmPtr->shmMsg[0] = 0;
                shmPtr->shmMsg[1] = 0;
                totProcCount++;
                //printf("currTime: %f\n", currTime);
                // if(currTime>=timeAlotted){
                //   printf("currTime higher. %f\n", currTime);
                // }else printf("timeAlotted higher. %f\n", timeAlotted);
                if((totProcCount>=100) || (sec >= 2) || (currTime>timeAlotted)){
                  if(currTime>=timeAlotted){
                    printf("Time Alotted Met: %f", currTime);
                  }
                  runMainLoop = 0;
                  //printf("totProcCount: %d\n", totProcCount);
                  //printf("sim time: %ld.%ld\n", sec, nans);
                  for(int x=0;x<=maxNumOfSlaves;x++){
                    kill(cpid[x], SIGQUIT);
                  }
                }else if(totProcCount<100 && sec != 2){  //implement the time check later; if(ossTime<2) && sysTime<specifiedTime

                  if((cpid[i] = fork()) == 0){
                    ChildProcess();
                  }else if(cpid[i]<0){
                    printf("\tError forking child");
                  }
                }
              }
            }
          }
          //printf("Going Going...\n");

          // if(working == 0){
          //   printf(".\n");
          //   working = (working+1)%3;
          // }else if (working == 1){
          //   printf(". .\r");
          //   working = (working + 1)%3;
          // }else{
          //   printf(". . . \r");
          //   working = (working +1)%3;
          // }

        shmPtr->nanoseconds += loopAdd;

        if((nans % billion) != nans){
          shmPtr->seconds++;
          shmPtr->nanoseconds = nans % billion;
        }
      }
      printf("Done\n");
      sleep(1);
      shmdt(shmPtr);					//must detach the shared memory
      shmctl(id, IPC_RMID, NULL);   //setting the control of the shared memory using the variable id = shmget(key,sizeof(shm), IPC_CREAT | 0666);
      fclose(file_ptr);

    }//runIt END

    //----------------------------------------------------------------------------------------------------
    if (sem_unlink(SEM_NAME) < 0)
        perror("sem_unlink(3) failed");

    return 0;
}


void ChildProcess(void){
    // char *args[]={"./user",NULL};
    // execvp(args[0],args);

    if (execl(CHILD_PROGRAM, CHILD_PROGRAM, NULL) < 0) {
      perror("execl(2) failed");
      exit(EXIT_FAILURE);
    }
}

void ctrlPlusC(int sig){

    fprintf( stderr, "Child %ld is dying from parent\n", (long)getpid());
    shmdt(shmPtr);
    sem_unlink(SEM_NAME);
    exit(1);
}
