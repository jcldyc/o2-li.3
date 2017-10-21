#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <errno.h>
#include <assert.h>
 
#include <unistd.h>
#include <sched.h>
 
pid_t parent_pid;
 
void sigquit_handler (int sig) {
    assert(sig == SIGQUIT);
    pid_t self = getpid();
    if (parent_pid != self) _exit(0);
}
 
int main ()
{
    int i;
 
    signal(SIGQUIT, sigquit_handler);
    parent_pid = getpid();
 
    puts("launching children");
    fflush(stdout);
    for (i = 0; i < 5; ++i) {
        pid_t p = fork();
        switch (p) {
        case 0:
            printf("child running: %d\n", (int)getpid());
            fflush(stdout);
            sleep(100);
            abort();
        case -1:
            perror("fork");
            abort();
        default:
            break;
        }
    }
 
    puts("sleeping 1 second");
    sleep(1);
 
    puts("killing children");
    kill(-parent_pid, SIGQUIT);
    for (i = 0; i < 5; ++i) {
        int status;
        for (;;) {
            pid_t child = wait(&status);
            if (child > 0 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
                printf("child %d succesully quit\n", (int)child);
            } else if (child < 0 && errno == EINTR) {
                continue;
            } else {
                perror("wait");
                abort();
            }
            break;
        }
    }
 
    puts("sleeping 5 seconds");
    sleep(5);
 
    puts("done");
    return 0;
}
