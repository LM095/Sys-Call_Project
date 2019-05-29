#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>		//lib per funzione upper()

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DIM_STRING 255 //dimensione massima della stringa--> scelta progettuale 

void stringInput(char* msg, char* str, int dim_string); //funzione per inserimento stringhe input
bool isServiceValid(char *str); 						//funzione che controlla il service (confronto stringhe)

struct Request
{
	char id[DIM_STRING + 1];
	char service[DIM_STRING + 1];
};

struct Response
{
	int key;
};

int main(void)
{
	struct Request clientRequest;
	char id[DIM_STRING + 1] = {""};
	char service[DIM_STRING + 1] = {""};

	//-------------- RICHIESTA DATI
	stringInput("Inserire l'id: \n", id, DIM_STRING);
	do
	{
		stringInput("\nInserire il servizio desiderato: Stampa, Salva, Invia\n", service, DIM_STRING);
		if(!isServiceValid(service))
			printf("Errore! Servizio non valido");	
	}
	while (!isServiceValid(service));
	
	strncpy(clientRequest.id,id, DIM_STRING);
	strncpy(clientRequest.service,service, DIM_STRING);

	printf("\nUser: %s\n",clientRequest.id);
	printf("Service: %s\n",clientRequest.service);

	//--------------- APERTURA FIFOCLIENT

	//inizio pezzo nuovo 24 maggio

    // Step-1: The client makes a FIFO
	// make a FIFO with the following permissions:
    // user:  read, write
    // group: write
    // other: no permission
    if (mkfifo("FIFOCLIENT", S_IRUSR | S_IWUSR | S_IWGRP) == -1)
    	printf("mkfifo failed");
	
	printf("<Client> FIFO %s created!\n", "FIFOCLIENT");

    // Step-2: The client opens the server's FIFO to send a Request
	printf("<Client> opening FIFO %s...\n", "FIFOSERVER");
    int serverFIFO = open("FIFOSERVER", O_WRONLY);		//in sola scrittura
    if (serverFIFO == -1)
        printf("open failed");

	// Step-3: The client sends a Request through the server's FIFO
    printf("<Client> sending %s\n", clientRequest.id);
    if (write(serverFIFO, &clientRequest,
        sizeof(struct Request)) != sizeof(struct Request))
        printf("write failed");

	// Step-4: The client opens its FIFO to get a Response
    int clientFIFO = open("FIFOCLIENT", O_RDONLY);		//in sola lettura
    if (clientFIFO == -1)
        printf("open failed");

    // Step-5: The client reads a Response from the server
    struct Response serverResponse;
    if (read(clientFIFO, &serverResponse,
        sizeof(struct Response)) != sizeof(struct Response))
        printf("read failed");

    // Step-6: The client prints the result on terminal
    printf("<Client> The server sent the result: %d\n", serverResponse.key);

    // Step-7: The client closes its FIFO
	if (close(serverFIFO) != 0 || close(clientFIFO) != 0)
        printf("close failed");

    // Step-8: The client removes its FIFO from the file system
    if (unlink("FIFOCLIENT") != 0)
        printf("unlink failed");

	//fine pezzo nuovo 24 maggio
	



	//fifo prova
	/*int fifoclient = open("fifoclient", O_WRONLY); //Returns file descriptor on success, or -1 on error. Si apre in sola scrittura, tanto il client deve solo scrivere
	if(fifoclient == -1)
		printf("Unable to open fifoclient");
	
	//--- write
    if(write(fifoclient, &clientRequest, sizeof(clientRequest)) == -1)    //riguardare il sizeof forse ci vuole qualche asterisco
        printf("Can't write\n");

    close(fifoclient);*/

	/*int dim=sizeof(clientRequest);
	printf("La dimensione di clientRequest è: %i\n",dim);
	return 0;*/
}


bool isServiceValid(char *str)
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

	if(strcmp(upper, "STAMPA") == 0 || strcmp(upper, "SALVA") == 0 || strcmp(upper, "INVIA") == 0)
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


