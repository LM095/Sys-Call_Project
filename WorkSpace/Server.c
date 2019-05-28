#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>		//lib per funzione upper()

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DIM_STRING 255 //dimensione massima della stringa--> scelta progettuale 
#define DIM_KEY 31

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
bool isServiceValid(char *str);

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
        if (byteRead == -1) 
        {
            printf("<Server> it looks like the FIFO is broken\n");
        } 
        else if (byteRead != sizeof(struct Request) || byteRead == 0)
            printf("<Server> it looks like I did not receive a valid request\n");
        else
        {
            sendResponse(&clientRequest, i);
        }

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
    if(serverFIFO != 0 && close(serverFIFO) == -1)
        printf("close failed");

    if(serverFIFO_extra != 0 && close(serverFIFO_extra) == -1)
        printf("close failed");

    // Remove the FIFO
    if(unlink(path2ServerFIFO) != 0)
        printf("unlink failed");

    // terminate the process
    _exit(0);
}

void sendResponse(struct Request *clientRequest, int i) 
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
        serverResponse.key = hashFunction(clientRequest->service);   //1234567890 + i;  
    else
        serverResponse.key = -1;

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

    while(*str != '\0')	
	{
    	upper[i] = toupper(*str);	//stringa maiuscola
		str++;						//artimetica puntatori
		i++;
	}
	upper[i] = '\0';

    strcpy(str, upper);
}

int sumAscii(char *str) {
    //trasformo in maiuscolo il service
    toUpperCase(str);

    //vado a calcolare la somma della singole lettere che compongono il service in decimale ASCII
    int j = 0;
    int sum = 0;
    while (str[j] != '\0')
    {
        sum += str[j];
        j++;
    }
    return sum;
}

int hashFunction(char *service)
{
    //vado a trovare la base della mia chiave sommando il valore decimale corrispondente.
    toUpperCase(service);
    
    char key[DIM_KEY + 1]= {""};  //31+1
    int i = 0;

    key[31] = '\0'; //boh
    
    while(i < 3)    //primi 3 char di service
    {
        key[i] = service[i];
        i++;
    }
    
    //finito questo while abbiamo dentro key i primi 3 caratteri del servizio richiesto
    //vado a generare randomicamente i restanti 28 caratteri:
    srand(time(NULL));

    while(key)  //finchè key non finisce (key[i] != '\0')
    {
        key[i]= ('!' + rand() %67);  //33 + (rand 0-66) = 99 = c al max
        i++;
    }

    DA 33 A 99
    STccc
    8384999999 = 
unsigned long int
 [0,4294967295]

    return key;    
}

