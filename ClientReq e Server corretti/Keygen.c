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
#define DIM_KEY 31
#define STAMPA_MASK 1
#define SALVA_MASK 2
#define INVIA_MASK 3

struct Request
{
    char id[DIM_STRING + 1];
    char service[DIM_STRING + 1];
};

bool isServiceValid(char *str);
void stringInput(char* msg, char* str, int dim_string);
void toUpperCase(char *str);
unsigned int keyEncrypter(char *service);
int keyDecrypter(unsigned int key);

int main (void)
{
    struct Request clientRequest;
    char id[DIM_STRING + 1] = {""};
    char service[DIM_STRING + 1] = {""};
    unsigned int key = 0;

    //-------------- RICHIESTA DATI--------------------
    stringInput("Inserire l'id: \n", id, DIM_STRING);
    stringInput("\nInserire il servizio desiderato: Stampa, Salva, Invia\n", service, DIM_STRING);
    strncpy(clientRequest.id, id, DIM_STRING);
    strncpy(clientRequest.service, service, DIM_STRING);
   
    if(isServiceValid(clientRequest.service))
        key = keyEncrypter(clientRequest.service);
    else
        key = 0;

    printf("\nLa chiave rilasciata per il servizio %s è %u\n\n", clientRequest.service, key);

    int verifica = keyDecrypter(key);
    switch(verifica)
    {
        case 1:
        {
            printf("Il servizio richiesto è STAMPA\n");
            break;
        }

        case 2:
        {
            printf("Il servizio richiesto è SALVA\n");
            break;
        }
        
        case 3:
        {
            printf("Il servizio richiesto è INVIA\n");
            break;
        }
        
        default:
        {
            printf("La chiave inserita non è valida\n");
            break;
        }
    }
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

/*
    Key Generator

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