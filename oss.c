//#define _POSIX_C_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

void ChildProcess(void);

int main (int argc, char *argv[]){
  int option;
  int terminateTime =0;
  int maxNumOfSlaves = 5;             //5 is the default amount
  char logFile[] = "logFile";         //set the name of the default log file
  int custLF = 0;                     //flag to determine if user wants a custom named log file; else logFile
  int runIt = 1;                      //flag to determine if program should run.
  FILE* file_ptr;
  pid_t pid;


  //struct used to hold the seconds, nanoseconds, and shmMsg and reference in shared memory

  typedef struct ShmData{
    int seconds;
    int nanoseconds;
    char shmMsg[256];
  }shmData;



    while((option=getopt(argc, argv, "hs:l:t:")) != -1){
      switch(option){
        case 'h':               //this option shows all the arguments available
          runIt = 0;            //how to only asked.  Set to 0 *report to log file*
          printf(" \t./oss: \n\t\t[-s (Max Number of Slaves)] \n\t\t[-l (Log File Name)] \n\t\t[-t (Time to Terminate)] \n");
          break;
        case 's':               //allows the user to set the max number of slave processes
          maxNumOfSlaves = atof(optarg);
          printf("\tMax number of processes spawned: %d \n", maxNumOfSlaves);
          break;
        case 'l':               //option to name the logfile used
          custLF = 1;

          if(custLF){
            file_ptr = fopen(optarg, "w");
            printf("\tLog File used: %s\n", optarg);
          }
          break;
        case 't':               //set's the time until termination
          terminateTime = atof(optarg);
          printf("\tTermination time: %d. \n", terminateTime);
          break;
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
    printf("\tLog File name: %s\n", logFile);
  }
  printf("-----------------------------------------------------------------------\n\n");



    // for(int i = 0; i < maxNumOfSlaves; i++){
    //   pid = fork();
    //   if(pid<0){
    //     printf("\tError forking child; pid: %ld\n", (long)getpid());
    //   }else if(pid == 0){
    //     ChildProcess();
    //   }else ParentProcess();
    //
    // }
    int needMoreProc = 1;
    int processCount = 0;
    while(needMoreProc){
      if(processCount < maxNumOfSlaves){
        pid = fork();
        if(pid<0){
          printf("\tError forking child; pid: %ld\n", (long)getpid());
        }else if(pid == 0) ChildProcess();
      }else needMoreProc = 0;
      processCount++;

    }


}


void ChildProcess(void){
    // pid_t pid;
    // printf("\tChild process created; pid: %ld ppid: %ld\n", (long)getpid(), (long)getppid());
    // pid = getpid();
    // //int killSuccess = kill(pid,SIGTERM);
    // if(kill(pid, SIGTERM)){
    //   printf("\tSuccessfully killed child process; pid: %ld", (long)getpid());
    // }else printf("\tNo Kill; pid: %ld", (long)getpid());
    char *args[]={"./user",NULL};
    execvp(args[0],args);
}
