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

//CLIENT

int main()
{
    char a[100];
    char b[100];

    int client = open("prova", O_WRONLY); //Returns file descriptor on success, or -1 on error. Si apre in sola scrittura, tanto il client deve solo scrivere

    if(client == -1)    
        printf("Can't open the FIFO\n");

    //--------------- input a e b
    printf("Digita un numero (a): ");
    scanf("%s", a);

    printf("Digita un numero (b): ");
    scanf("%s", b);

    //------------- write
    if(write(client, a, 4) == -1)    //4 = sizeof(int)
        printf("Can't write\n");

    if(write(client, b, 4) == -1)    //4 = sizeof(int)
        printf("Can't write\n");

    close(client);

    return 0;
}