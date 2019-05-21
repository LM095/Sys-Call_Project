/*
Ese_1: (lab)
Scrivere un’applicazione Client-Server basata su FIFO. Il server crea una FIFO, e attende un messaggio. Il
messaggio e’ un vettore di 2 interi [a,b]. Se a < b, il server stampa: “a e’ minore di b”; se a >= b il server
stampa: “a e’ maggiore o uguale di b”. Dopo aver stampato a video la stringa, il server rimuove la FIFO,
ed infine termina. Il client chiede all’utente due interi, invia i due numeri al server tramite la FIFO ed
infine termina
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

//SERVER

int main()
{
    //int valori[] = {a, b};

    int coda = mkfifo("prova", S_IRUSR | S_IWUSR); //Returns 0 on success, or -1 on error. Permessi: read user, write user

    if(coda == -1)
        printf("Can't create a FIFO\n");

    int server = open("prova", O_RDONLY); //Returns file descriptor on success, or -1 on error. Si apre in sola lettura, tanto il server deve solo leggere

    if(server == -1)    
        printf("Can't open the FIFO\n");

    char valori[4]; //4 = sizeof(int)    

    //--------------------read
    if(read(server, valori, 4) == -1)   //4 = sizeof(int)  
        printf("Can't read\n");

    int a = atoi(valori);

    if(read(server, valori, 4) == -1)   //4 = sizeof(int)
        printf("Can't read\n");

    int b = atoi(valori);

    //--------------------print
    if(a < b)
        printf("a è minore di b!\n");
    else if(a >= b)
        printf("a è maggiore o uguale a b\n");

    unlink("prova");

    return 0;
}