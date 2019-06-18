#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <sys/types.h>

#include "semaphore.h"
#include "shmem.h"

/************* PREPROCESSOR DEFINE *************/

#define DIM_STRING 255              //max dim of Strings 
#define STAMPA_MASK 1
#define SALVA_MASK 2
#define INVIA_MASK 3
#define TABLE_SIZE 1024

/************* FUNCTIONS DEFINITIONS *************/

int keyDecrypter(unsigned int key);
bool isValidKey(char id[], unsigned int key, int size);
void signalsHandler(int sig); 
void printTitle();

/*************  GLOBAL VARIABLES *************/

int semid;
int shmidClientExec;
pid_t keyManager;
struct keyTable *table;

int main(int argc, char *argv[])
{
    printTitle();
    char id[DIM_STRING + 1] = {0};
    unsigned int key = 0;
    int service = 0;
    
    if(argc < 4)
    {
        printf("Expected: user_id, key, at least one argument for the service\n");
        return 0;
    }       
    
    printf("<ClientExec> Creating a semaphore...\n");
    semid = getSemSet(SEM_KEY);
   
    /************* SHARED MEMORY *************/
    size_t size = sizeof(struct keyTable) * TABLE_SIZE;

    printf("<ClientExec> Getting a shared memory segment...\n");
    shmidClientExec = allocSharedMemory(SHM_KEY, size);
    table = (struct keyTable*)getSharedMemory(shmidClientExec, 0);

    if(signal(SIGTERM, signalsHandler) == SIG_ERR) //catch sigterm to terminate gracefully and deatach the shmem/sem 
        printf("\nProblem setting Sigterm Handler\n");
        
    /************* PARAM RECEIVER *************/
    strcpy(id, argv[1]);
    key = strtoul(argv[2], NULL, 10);   //str to unsigned long

    /************* SHARED MEMORY CHECK *************/    
    if(!isValidKey(id, key, TABLE_SIZE))
    {
        printf("Invalid key for the requested service\n");
        freeSharedMemory(table);
        return 0;
    }
    else
    {       
        freeSharedMemory(table);

        service = keyDecrypter(key);
        argv += 3;  //exclude the program name, user, key

        /************* MAIN PROGRAM *************/
        switch(service)
        {
            case STAMPA_MASK:
            {          
                printf("<ClientExec> Calling Stampa program...\n");      
                execv("stampa", argv);
                break;
            }
            case SALVA_MASK:
            {
                printf("<ClientExec> Calling Salva program...\n");  
                execv("salva", argv);
                break;
            }
            case INVIA_MASK:
            {
                printf("<ClientExec> Calling Invia program...\n");  
                execv("invia", argv);
                break;
            }
            default:
            {
                printf("Invalid service\n");
                return 0;
            }
        }
    }
   
    return 0;
}

bool isValidKey(char id [], unsigned int key, int size)
{
    int i = 0;
    bool result;
    
    pMutex(semid, 0);

    for(i = 0; i < size ; i++)
    {
        if((strcmp(table[i].user, id) == 0) && table[i].key == key)
        {
            result = true;
            
            //delete key from shared memory
            strcpy(table[i].user, "");      
            table[i].key = 0;
            table[i].timestamp = 0;

            break;
        }
    }  

    if(i == size)
        result = false;

    vMutex(semid, 0);
    
    return result;
}

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

void quit() 
{    
    /************* CLOSE SHARED MEMORY AND SEMAPHORE *************/
    freeSharedMemory(table); 
    removeSemaphore(semid);   
}

void signalsHandler(int sig) 
{
    switch(sig)
    {
        case SIGTERM:
        {   
            quit();     //close shared memory and semaphore.
            exit(0);
            break;
        }
        default:
        {
            break;
        }
    }         
}

void printTitle()
{
    printf(
		"\n" 												
		"   _____ _ _            _     ______               	  \n"
		"  / ____| (_)          | |   |  ____|                    \n"  
		" | |    | |_  ___ _ __ | |_  | |__  __  _____  ___       \n"  
		" | |    | | |/ _ \\ '_ \\| __| |  __| \\ \\/ / _ \\/ __| \n"  
		" | |____| | |  __/ | | | |_  | |____ >  <  __/ (__.      \n"   
		"  \\_____|_|_|\\___|_| |_|\\__| |______/_/\\_\\___|\\___|\n"         
		" \n\n");
}
