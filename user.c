#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdbool.h>
#include <sys/types.h>
#include <unistd.h>
#include <signal.h>

int main (int argc, char *argv[]){
  pid_t pid = getpid();

  printf("inside of user process;  pid: %ld\n", (long)pid);
  kill(pid, SIGTERM);
  return 0;

}
