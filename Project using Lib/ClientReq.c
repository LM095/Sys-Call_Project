#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

/************* PREPROCESSOR DEFINE *************/

#define DIM_STRING 255 		//max dim of Strings 

/************* FUNCTIONS DEFINITIONS *************/

void stringInput(char* msg, char* str, int dimString);
void closeFifos(int serverFIFO, int clientFIFO);
void removeFifo(char *path2ClientFIFO);
void printTitle();

/************* STRUCT *************/

struct Request
{
	char id[DIM_STRING + 1];
	char service[DIM_STRING + 1];
};

struct Response
{
	unsigned int key;
};

int main(void)
{
	struct Request clientRequest;	
	struct Response serverResponse;
	char id[DIM_STRING + 1] = {0};
	char service[DIM_STRING + 1] = {0};
    char path2ClientFIFO [256];
	char *path2ServerFIFO = "FIFOSERVER";
	char *baseClientFIFO = "FIFOCLIENT.";
	int serverFIFO;
	int clientFIFO;

	/************* DATA INPUT *************/
	printTitle();
	stringInput("Hi, I'm ClientReq program!\nInsert your ID: \n", id, DIM_STRING);
	stringInput("\nInsert the desired service: \n-Stampa\n-Salva\n-Invia\n", service, DIM_STRING);
	
	strncpy(clientRequest.id,id, DIM_STRING);
	strncpy(clientRequest.service,service, DIM_STRING);

	/************* FIFO *************/
    sprintf(path2ClientFIFO, "%s%s", baseClientFIFO, id);

    if (mkfifo(path2ClientFIFO, S_IRUSR | S_IWUSR | S_IWGRP) == -1)	
		printf("Failed making Fifo\n");    		
	
	printf("<Client> FIFO %s created!\n", path2ClientFIFO);

	printf("<Client> Opening FIFO %s...\n", path2ServerFIFO);
    serverFIFO = open(path2ServerFIFO, O_WRONLY);

    if (serverFIFO == -1)
        printf("Failed opening Fifo %s\n", path2ServerFIFO);	

    printf("<Client> Sending %s to Server\n", clientRequest.id);
	
    if (write(serverFIFO, &clientRequest, sizeof(struct Request)) != sizeof(struct Request))
        printf("Failed writing\n");

    clientFIFO = open(path2ClientFIFO, O_RDONLY);
    if (clientFIFO == -1)
        printf("Failed opening Fifo\n");
    
    if (read(clientFIFO, &serverResponse, sizeof(struct Response)) != sizeof(struct Response))
        printf("Failed reading\n");

	/************* KEY RECEIVED *************/
    printf("<Client> Server sent you the key: %u\n", serverResponse.key);

	/************* CLOSING FIFO *************/
	closeFifos(serverFIFO, clientFIFO);
	removeFifo(path2ClientFIFO);
}

/************* FUNCTIONS IMPLEMENTATIONS *************/

void stringInput(char* msg, char* str, int dimString)
{
	char input[DIM_STRING + 1] = {""};

	printf("%s", msg);
	scanf(" %255[^\n]s", input);			//regex to take spaces, 255 char at max and to ignore the \n
	strncpy(str, input, dimString);
}

void closeFifos(int serverFIFO, int clientFIFO)
{
	if (close(serverFIFO) != 0 || close(clientFIFO) != 0)
        printf("Failed closing Fifo\n");		
}

void removeFifo(char *path2ClientFIFO)
{
    if (unlink(path2ClientFIFO) != 0)
        printf("Failed unlinking Fifo\n");
}

void printTitle()
{
	printf(
		"\n" 		
		"  _____  _  _               _     _____                                 _   \n"
		" / ____|| |(_)             | |   |  __ \\                               | |  \n"
		"| |     | | _   ___  _ __  | |_  | |__) | ___   __ _  _   _   ___  ___ | |_ \n"
		"| |     | || | / _ \\| '_ \\ | __| |  _  / / _ \\ / _` || | | | / _ \\/ __|| __|\n"
		"| |____ | || ||  __/| | | || |_  | | \\ \\|  __/| (_| || |_| ||  __/\\__ \\| |_ \n"
		" \\_____||_||_| \\___||_| |_| \\__| |_|   \\_\\___| \\__, | \\__,_| \\___||___/ \\__|\n"
		"                                                  | |\n"
		"                                                  |_|\n"
		"												   \n"
		" \n\n");
	
}