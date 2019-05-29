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

#define DIM_STRING 255 //dimensione massima della stringa--> scelta progettuale 
#define STAMPA_MASK 1
#define SALVA_MASK 2
#define INVIA_MASK 3

///////// STRUCT /////////////////////////

struct Request
{
	char id[DIM_STRING + 1];
	char service[DIM_STRING + 1];
};

struct Response
{
	int key;
};

///////// FUNCTIONS DEFINITIONS /////////

void quit();
void sendResponse(struct Request *request);
bool isServiceValid(char *str);
void stringInput(char* msg, char* str, int dim_string);
void toUpperCase(char *str);
unsigned int keyEncrypter(char *service);
int keyDecrypter(unsigned int key);

char *path2ServerFIFO = "FIFOSERVER";
char *baseClientFIFO = "FIFOCLIENT.";

// the file descriptor entry for the FIFO
int serverFIFO, serverFIFO_extra;

int main (void) 
{
    struct Request clientRequest;
    int byteRead = -1;
   
    printf("<Server> Making FIFO...\n");
    // make a FIFO with the following permissions:
    // user:  read, write
    // group: write
    // other: no permission
    if(mkfifo(path2ServerFIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)
    {
        printf("mkfifo failed");
    }
        
    printf("<Server> FIFO %s created!\n", path2ServerFIFO);

    // Wait for client in read-only mode. 
    // The "open" blocks the calling process
    // until another process opens the same FIFO in write-only mode
    printf("<Server> waiting for a client...\n");
    serverFIFO = open(path2ServerFIFO, O_RDONLY);      //sola lettura
    if(serverFIFO == -1)
        printf("open read-only failed");

    // non bloccante??????
    // Open an extra descriptor, so that the server does not see end-of-file
    // even if all clients closed the write end of the FIFO
    serverFIFO_extra = open(path2ServerFIFO, O_WRONLY);    //sola scrittura
    if(serverFIFO_extra == -1)
        printf("open write-only failed");

    do{
        printf("<Server> waiting for a Request...\n");
        // Read a request from the FIFO
        byteRead = read(serverFIFO, &clientRequest, sizeof(struct Request));

        // Check the number of bytes read from the FIFO
        if(byteRead == -1) 
        {
            printf("<Server> it looks like the FIFO is broken\n");
        } 
        else if (byteRead != sizeof(struct Request) || byteRead == 0)
            printf("<Server> it looks like I did not receive a valid request\n");
        else
            sendResponse(&clientRequest);
        
    }
    while (byteRead != -1);

    // the FIFO is broken, run quit() to remove the FIFO and
    // terminate the process.
    quit();

    return 0;
}

//////////// FUNCTIONS IMPLEMENTATIONS ///////////////////

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

void sendResponse(struct Request *clientRequest)
{
    struct Response serverResponse;
    int clientFIFO;
    char path2ClientFIFO [256];
    int byteWrite = 0;

    // make the path of client's FIFO    
    sprintf(path2ClientFIFO, "%s%s", baseClientFIFO, clientRequest->id);
    printf("<Server> opening FIFO %s...\n", path2ClientFIFO);
    // Open the client's FIFO in write-only mode

    clientFIFO = open(path2ClientFIFO, O_WRONLY);  //sola scrittura
    if (clientFIFO == -1) 
    {
        printf("<Server> open failed");
        return;
    }

    if(isServiceValid(clientRequest->service))
        serverResponse.key = keyGenerator(clientRequest->service);  //calls keyEncrypter and check if the key is unique. Returns the key
    else
        serverResponse.key = 0;

    // Prepare the response for the client   

    printf("<Server> sending a response\n");
    // Write the Response into the opened FIFO

    byteWrite = write(clientFIFO, &serverResponse, sizeof(struct Response));
        
    // Check the number of bytes written to the FIFO
    if (byteWrite == -1) 
    {
        printf("<Server> write failed");
        return;
    } 
    else if (byteWrite != sizeof(struct Response) || byteWrite == 0)
    {
        printf("<Server> it looks like I can't write all the fields\n");
        return;
    }
    
    //danese version
    /*if(write(clientFIFO, &serverResponse,sizeof(struct Response)) != sizeof(struct Response)) 
    {
        printf("<Server> write failed");
        return;
    }*/

    // Close the FIFO
    if(close(clientFIFO) != 0)
        printf("<Server> close failed");
}

bool isServiceValid(char *str)
{
    toUpperCase(str);

    if(strcmp(str, "STAMPA") == 0 || strcmp(str, "SALVA") == 0 || strcmp(str, "INVIA") == 0)
        return true;

    return false;
}

void stringInput(char* msg, char* str, int dim_string)
{
    char input[DIM_STRING + 1] = {""};

    printf("%s", msg);
    scanf(" %255[^\n]s", input);			//regex per prendere gli spazi, 255 char al max, e per ignorare i \n
    strncpy(str, input, dim_string);
}

void toUpperCase(char *str)
{
    char upper[DIM_STRING + 1] = {""};
    int i = 0;

    while(str[i] != '\0')
    {
        upper[i] = toupper(str[i]);	//stringa maiuscola
        i++;
    }
    upper[i] = '\0';

    strcpy(str, upper);
}

unsigned int keyGenerator(char *service)
{
    unsigned int key = 0;

    do
    {
        key = keyEncrypter(service); 
    }
    while(key ); //funzione che checkka in memoria condivisa e torna un bool per vedere se la chiave esiste

    /*
    - creare mem condivisa da ?? byte
    - come la rappresentiamo? con array? con lista?
    */

    return key;
}

/*
    Key Encrypter

    Param: a string that specifies the service (upper case)
    Return: the key generated as an unsigned int (2^32, all positive)

    The function creates a key using bitwise operators.
    First, a random is generated between 0 and UINT_MAX-1 ((2^32)-1), 
    then it's left shifted by 2 bits. 
    Now that the last 2 positions are 00, there's an OR operation 
    between the random and the respective mask service (constant), 
    that fills the last 2 positions.
    Finally, the key is returned.
*/
unsigned int keyEncrypter(char *service)
{
    srand(time(NULL));
    unsigned int key = 0;
    unsigned int random = rand() % UINT_MAX;       
    
    random = random << 2; 
    if(strcmp(service, "STAMPA") == 0)
        key = random | STAMPA_MASK;
    else if(strcmp(service, "SALVA") == 0)
        key = random | SALVA_MASK;
    else if(strcmp(service, "INVIA") == 0)
        key = random | INVIA_MASK;
    else
        return 0;

    return key;
}

/*
    Service decrypter

    Param: the key that needs to be decrypted
    Return: the service's respective mask

    The function takes the key, and uses the bitwise operation AND 
    to decrypt it. 
    If the mask is equals to the result, then the service is the mask itself.

    Note that it needs to be in that order (from the highest to the lowest) in order to work!
*/
int keyDecrypter(unsigned int key)
{
    if((key & INVIA_MASK) == INVIA_MASK)
    {
        return INVIA_MASK;
    }
    else if((key & SALVA_MASK) == SALVA_MASK)
    {
        return SALVA_MASK;
    }
    else if((key & STAMPA_MASK) == STAMPA_MASK)
    {
        return STAMPA_MASK;
    }
    else
        return 0;
}