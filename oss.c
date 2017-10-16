#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>

int main (int argc, char *argv[]){
  int option;
  int terminateTime =0;
  int maxNumOfSlaves = 5;            //5 is the default amount
  char logFile[] = "logFile";
  int custLF = 0;              //flag set to determine if user wants a custom named log file; else logFile
  FILE* file_ptr;



    while((option=getopt(argc, argv, "hs:l:t:")) != -1){
      switch(option){
        case 'h':               //this option shows all the arguments available
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
    if(argc < 3){
      printf("\tDefault arguments used. \n");
    }

  //create the log file specified by user
  //otherwise, use the default log file name : logFile

  if(!custLF){
    file_ptr = fopen(logFile, "w");
    printf("\tLog File name: %s\n", logFile);
  }








}
