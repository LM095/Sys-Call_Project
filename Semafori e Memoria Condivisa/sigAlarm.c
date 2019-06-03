#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>		//lib per funzione upper()
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>

int i = 0;

void sigHandler(int sig)
{
    i = 1;
    printf("Out of time!\n"); _exit(0);
}

int main (int argc, char *argv[])
{
    if (signal(SIGALRM, sigHandler) == SIG_ERR)
        printf("change signal handler failed");

    int time = 2;
    printf("We have %d seconds to complete the job!\n", time);
    alarm(time); // setting a timer

    while(i == 0)
    {
    }

    time = alarm(0); // disabling timer
    printf("%d seconds before timer expirations!\n", time);
}