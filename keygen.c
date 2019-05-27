#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>		//lib per funzione upper()
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>

#define DIM_STRING 255

struct Request
{
    char id[DIM_STRING + 1];
    char service[DIM_STRING + 1];
};

struct Response
{
    int key;
};

struct memoryAllocation
{
    char user[DIM_STRING + 1];
    int  key;
    time_t timestamp;

};

///////////////////////////DICHIARAZIONE FUNZIONI ////////////////////////////////////////////////
void stringInput(char* msg, char* str, int dim_string); //funzione per inserimento stringhe input
bool isServiceValid(char *str);
int hashFunction(char service[],int key_generator);
int sumAscii(char string_to_convert[]);
//////////////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////// MAIN ///////////////////////////



int main(void)
{
    struct Request clientData;
    char id[DIM_STRING + 1] = {""};
    char service[DIM_STRING + 1] = {""};
    stringInput("Inserire l'id: \n", id, DIM_STRING);
    stringInput("\nInserire il servizio desiderato: Stampa, Salva, Invia\n", service, DIM_STRING);
    strncpy(clientData.id,id, DIM_STRING);
    strncpy(clientData.service,service, DIM_STRING);

    printf("\nUser: %s\n",clientData.id);
    printf("Service: %s\n",clientData.service);

    //IL SERVER AVRA' SEMPRE IL KEYGENERATOR E LO PASSERA' PER CREARE LA NUOVA KEY:
    int key_generator = 0;
    int chiave = hashFunction(clientData.service,key_generator);
    printf("la chiave rilasciata è: %i\n\n", chiave);
    

    return 0;
}



///////////////////////////IMPLEMENTAZIONE FUNZIONI///////////////////////////

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

int sumAscii(char str[]) {
    //trasformo in maiuscolo il service
    char upper[DIM_STRING + 1] = {""};
    int i = 0;
    while(*str != '\0')
    {
        upper[i] = toupper(*str);	//stringa maiuscola
        str++;						//artimetica puntatori
        i++;
    }
    upper[i] = '\0';

    //vado a calcolare la somma della singole lettere che compongono il service in decimale ASCII
    int j = 0;
    int sum = 0;
    while (upper[j] != '\0')
    {
        sum += upper[j];
        j++;
    }
    return sum;
}


int hashFunction(char service[],int key_generator)
{
    char temp[] = "";
    if (!isServiceValid(service)) //se è un servizio non valido resistituisco -1 e termino
        return -1;
    else
    {
        int key = 0;
        //vado a trovare la base della mia chiave sommando il valore decimale corrispondente.
        int base_key = sumAscii(service);
        key= base_key + key_generator;
        return key;
    }
}

