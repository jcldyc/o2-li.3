#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>

int main (int argc, char *argv[]){
  int option;
  int maxNumOfSlaves = 5;          //5 is the default amount


  //if(argc>2){
    while((option=getopt(argc, argv, "hs:l:t:")) != -1){
      switch(option){
        case 'h':
          printf(" \t./oss: \n\t\t[-s (Max Number of Slaves)] \n\t\t[-l (Log File Name)] \n\t\t[-t (Time to Terminate)] \n");
          break;
        case 's':
          maxNumOfSlaves = atof(optarg);
          printf("\tMax number of processes spawned: %d \n", maxNumOfSlaves);
          break;
        case 'l':
          FILE* file_ptr = fopen(optarg, "w");
          printf("\tLog File used: %s\n", optarg);
          break;
        case 't':
          printf("\toption t \n");
          break;
        default:
          printf("\tno option used \n");
          break;
      }
    }
  //}else printf("\tDefault arguments used. \n");


  //create the log file specified by user
  //otherwise, use the default log file name : logFile
  FILE* file_ptr = fopen("logFile", "w");



}
