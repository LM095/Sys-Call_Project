#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>

#define DIM_STRING 255 //dimensione massima della stringa--> scelta progettuale 

void stringInput(char* msg, char* str, int dim_string); //funzione che permette l'inserimento degli input
bool isServiceValid(char *str); //funzione che controllo il service (confronto stringhe)

struct request
{
	char id[DIM_STRING + 1];
	char service[DIM_STRING + 1];
};

int main(void)
{
	struct request r1;
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
	
	strncpy(r1.id,id, DIM_STRING);
	strncpy(r1.service,service, DIM_STRING);

	printf("\nUser: %s\n",r1.id);
	printf("Service: %s\n",r1.service);
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


