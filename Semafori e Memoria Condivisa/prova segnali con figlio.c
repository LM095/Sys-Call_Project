
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
#include <signal.h>

#define ALARM_TIME 2

pid_t pid = 0;      //proviamo globale

void signalsHandler(int sig)
{
    switch(sig)
    {
        case SIGALRM:   //è solo il figlio
        {
            printf("Pid che chiama sig alarm (deve essere quello del figlio!!): %i\n", getpid()); //deve essere quello del figlio!!!
            alarm(ALARM_TIME);

            if(signal(SIGALRM, signalsHandler) == SIG_ERR) 
                printf("\nProblema");

            break;
        }
        case SIGTERM:
        {
            printf("Sono %i e sto per killare qualcosa\n", getpid());

            if(pid == 0)                    //ovvero è il figlio
            {
                exit(0);                     //mi suicido
                //kill(getpid(), SIGQUIT);    //mi suicido
            }
            else                            //ovvero il pid tornato è quello del figlio!
            {
                kill(pid, SIGTERM);         //figlio ko
                exit(0);
                //kill(getpid(), SIGTERM);    //mi suicido (il padre)
            }

            break;
        }
    }
}

int main(void)
{
    sigset_t setSegnali, vecchioSetSegnali, maskFiglio;

    /*
    //tutta la maschera a 0
    sigemptyset(&setSegnali);

    //aggiungo il segnale SIGTERM
    sigaddset(&setSegnali, SIGTERM);

    //sblocco solo sigterm
    sigprocmask(SIG_SETMASK, &setSegnali, &vecchioSetSegnali);
    */
    
    //tutta la maschera a 1
    sigfillset(&setSegnali);

    //tolgo il segnale SIGTERM
    sigdelset(&setSegnali, SIGTERM);    //non blocca SOLO sigterm
    //sigdelset(&setSegnali, SIGALRM);
    //sigdelset(&setSegnali, SIGSTOP);

    //sblocco solo sigterm
    sigprocmask(SIG_SETMASK, &setSegnali, &vecchioSetSegnali);

    if(signal(SIGTERM, signalsHandler) == SIG_ERR) 
        printf("\nProblema");

    printf("Pid padre da parte del padre: %i\n", getpid());

    pid = fork();

    switch(pid)
    {
        case -1:
        {
            printf("Errore!!!\n");
            break;
        }
        case 0:     //figlio
        {
            printf("Pid padre da parte del figlio: %i\n", getppid());
            printf("Pid figlio da parte del figlio: %i\n", pid);

            //sigdelset(&maskFiglio, SIGALRM);    //non blocca sigalarm
            //sigprocmask(SIG_BLOCK, &maskFiglio, &setSegnali); //boh

            sigdelset(&setSegnali, SIGALRM);    //non blocca sigalarm

            /*The last argument, oldset, is used to return information about the old process signal mask. 
            If you just want to change the mask without looking at it, pass a null pointer as the oldset argument. */
            sigprocmask(SIG_SETMASK, &setSegnali, NULL); //boh

            printf("\n - Imposto un timer di %i secondi finchè non arriva sigterm\n", ALARM_TIME);

            if(signal(SIGALRM, signalsHandler) == SIG_ERR) 
                printf("\nProblema");

            alarm(ALARM_TIME);

            //if(signal(SIGTERM, signalsHandler) == SIG_ERR) 
                //printf("\nProblema");

            while(1)            
                pause();    //boh

            break;
        }
        default:    //padre
        {
            printf("Pid figlio da parte del padre: %i\n", pid);

            //if(signal(SIGTERM, signalsHandler) == SIG_ERR) 
                //printf("\nProblema");

            while(1)            
                pause();

            break;
        }
    }
    
    //while(1);

    return 0;
}