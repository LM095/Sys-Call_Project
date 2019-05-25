#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>		//lib per funzione upper()

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DIM_STRING 255 //dimensione massima della stringa--> scelta progettuale 

struct Request
{
	char id[DIM_STRING + 1];
	char service[DIM_STRING + 1];
};

struct Response
{
	int key;
};

void quit();
void sendResponse(struct Request *request,int i);

char *path2ServerFIFO = "FIFOSERVER";
char *baseClientFIFO = "FIFOCLIENT.";

// the file descriptor entry for the FIFO
int serverFIFO, serverFIFO_extra;

int main (void) 
{
   
    printf("<Server> Making FIFO...\n");
    // make a FIFO with the following permissions:
    // user:  read, write
    // group: write
    // other: no permission
    if (mkfifo(path2ServerFIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
    {
        printf("mkfifo failed");
    }
        

    printf("<Server> FIFO %s created!\n", path2ServerFIFO);

    // Wait for client in read-only mode. 
    // The "open" blocks the calling process
    // until another process opens the same FIFO in write-only mode
    printf("<Server> waiting for a client...\n");
    serverFIFO = open(path2ServerFIFO, O_RDONLY);      //sola lettura
    if (serverFIFO == -1)
        printf("open read-only failed");

    // non bloccante??????
    // Open an extra descriptor, so that the server does not see end-of-file
    // even if all clients closed the write end of the FIFO
    serverFIFO_extra = open(path2ServerFIFO, O_WRONLY);    //sola scrittura
    if (serverFIFO_extra == -1)
        printf("open write-only failed");

    struct Request clientRequest;
    int byteRead = -1;
    int i=0;
    do {
        printf("<Server> waiting for a Request...\n");
        // Read a request from the FIFO
        byteRead = read(serverFIFO, &clientRequest, sizeof(struct Request));

        // Check the number of bytes read from the FIFO
        if (byteRead == -1) {
            printf("<Server> it looks like the FIFO is broken\n");
        } else if (byteRead != sizeof(struct Request) || byteRead == 0)
            printf("<Server> it looks like I did not receive a valid request\n");
        else
            sendResponse(&clientRequest,i);
        i++;
    } while (byteRead != -1);

    // the FIFO is broken, run quit() to remove the FIFO and
    // terminate the process.
    quit();

    return 0;
}

void quit() 
{
    // Close the FIFO
    if (serverFIFO != 0 && close(serverFIFO) == -1)
        printf("close failed");

    if (serverFIFO_extra != 0 && close(serverFIFO_extra) == -1)
        printf("close failed");

    // Remove the FIFO
    if (unlink(path2ServerFIFO) != 0)
        printf("unlink failed");

    // terminate the process
    _exit(0);
}

void sendResponse(struct Request *clientRequest,int i) 
{
    // make the path of client's FIFO
    char path2ClientFIFO [256];
    sprintf(path2ClientFIFO, "%s%s", baseClientFIFO, clientRequest->id);
    printf("<Server> opening FIFO %s...\n", path2ClientFIFO);
    // Open the client's FIFO in write-only mode
    int clientFIFO = open(path2ClientFIFO, O_WRONLY);  //sola scrittura
    if (clientFIFO == -1) {
        printf("<Server> open failed");
        return;
    }

    // Prepare the response for the client
    struct Response serverResponse;
    serverResponse.key = 1234567890 + i;

    printf("<Server> sending a response\n");
    // Wrinte the Response into the opened FIFO
    if (write(clientFIFO, &serverResponse,
              sizeof(struct Response)) != sizeof(struct Response)) {
        printf("<Server> write failed");
        return;
    }

    // Close the FIFO
    if (close(clientFIFO) != 0)
        printf("<Server> close failed");
}