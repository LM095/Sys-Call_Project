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

int main(void)
{
	struct Request clientData;
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
	
	strncpy(clientData.id,id, DIM_STRING);
	strncpy(clientData.service,service, DIM_STRING);

	printf("\nUser: %s\n",clientData.id);
	printf("Service: %s\n",clientData.service);

	// prova struct fifo

	struct Request *provastruct= (struct Request*)malloc(sizeof(struct Request));
	strcpy(provastruct->id, "culo");
	strcpy(provastruct->service, "stampa");

	//--------------- APERTURA FIFOCLIENT
	int fifoclient = open("fifoclient", O_WRONLY); //Returns file descriptor on success, or -1 on error. Si apre in sola scrittura, tanto il client deve solo scrivere
	if(fifoclient == -1)
		printf("Unable to open fifoclient");
	
	//--- write
    if(write(fifoclient, provastruct, sizeof(*provastruct)) == -1)    //4 = sizeof(int)
        printf("Can't write\n");

    close(fifoclient);

	//--------------- APERTURA FIFOCLIENT
	/*int fifoclient = open("fifoclient", O_WRONLY); //Returns file descriptor on success, or -1 on error. Si apre in sola scrittura, tanto il client deve solo scrivere
	if(fifoclient == -1)
		printf("Unable to open fifoclient");
	
	//--- write
    if(write(fifoclient, clientData, sizeof(clientData)) == -1)    //4 = sizeof(int)
        printf("Can't write\n");

    close(fifoclient);*/


	int dim=sizeof(clientData);
	printf("La dimensione di rclientData1 è: %i\n",dim);
	return 0;
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


