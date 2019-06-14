#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>		//lib for Upper() fun
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
#define USED_KEY_ARRAY_SIZE 2048    //twice the shared mem
#define TIME_ALARM 30

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

/************* FUNCTIONS DEFINITIONS *************/

void quit(); 
unsigned int updateTable(struct Request request, unsigned int usedKeys[], int tableSize, int usedKeysSize);
bool isUniqueKey(unsigned int key, unsigned int usedKeys[], int tableSize, int usedKeysSize);
void sendResponse(struct Request *request, unsigned int key);
bool isServiceValid(char *str);
void toUpperCase(char *str);
unsigned int keyEncrypter(char *service);
void signalsHandler(int sig); 
void checkTableEvery30Sec(int size);
void keyManagerFun(sigset_t setSig);
void serverFun(struct Request clientRequest, unsigned int usedKeys[]);
void closeFifos();
void removeFifo();

/*************  GLOBAL VARIABLES *************/

char *path2ServerFIFO = "FIFOSERVER";
char *baseClientFIFO = "FIFOCLIENT.";
int semid;
int shmidServer;
pid_t keyManager;
struct keyTable *table;
int serverFIFO, serverFIFO_extra;   // the file descriptor entry for the FIFO

int main (void) 
{
    struct Request clientRequest;
    sigset_t setSig; 
    unsigned int usedKeys[TABLE_SIZE] = {0};    //contains old keys
    size_t size;                                //table size

    /************* FIFO *************/

    printf("<Server> Making FIFO...\n");
    if(mkfifo(path2ServerFIFO, S_IRUSR | S_IWUSR) == -1)
        printf("Failed making FIFO\n");
        
    printf("<Server> FIFO %s created!\n", path2ServerFIFO);

    printf("<Server> Waiting for a client...\n");
    serverFIFO = open(path2ServerFIFO, O_RDONLY);
    if(serverFIFO == -1)
        printf("Open read-only failed\n");

    //Extra descriptor: so the server does not see EOF
    serverFIFO_extra = open(path2ServerFIFO, O_WRONLY);   
    if(serverFIFO_extra == -1)
        printf("Open write-only failed\n");

    /************* SYNC *************/

    /************* SIGNALS *************/

    sigfillset(&setSig);
    sigdelset(&setSig, SIGTERM);                //free only SIGTERM
    sigprocmask(SIG_SETMASK, &setSig, NULL);

    if(signal(SIGTERM, signalsHandler) == SIG_ERR) 
        printf("\nProblem setting Sigterm Handler\n");

    /************* SEMAPHORE *************/
    printf("<Server> Creating a semaphore and setting to 0...\n");
    semid = createSemSet(SEM_KEY);

    /************* SHARED MEMORY *************/
    size = sizeof(struct keyTable) * TABLE_SIZE;

    printf("<Server> Allocating a shared memory segment...\n");
    shmidServer = allocSharedMemory(SHM_KEY, size);
      
    table = (struct keyTable*)getSharedMemory(shmidServer, 0);    //getting pointer table sh mem

    vMutex(semid, 0); 
    
    //Server's pid
    printf("<Server> Server's PID: %i\n", getpid());

    /************* FORK *************/
    keyManager = fork();    

    /************* MAIN PROGRAM *************/
    switch(keyManager)  
    {
        case -1:
        {
            printf("\nError fork\n");
            break;
        }
        case 0:     //keyManager
        {
            keyManagerFun(setSig);
            break;
        }
        default:    //server
        {            
            serverFun(clientRequest, usedKeys);
            break;
        }
    }    
    
    return 0;
}

/************* FUNCTIONS IMPLEMENTATIONS *************/

void keyManagerFun(sigset_t setSig)
{
    printf("<Key Manager> KeyManager's PID: %i\n", getpid());
    sigdelset(&setSig, SIGALRM);                //free SIGALRM only for keymanager (inherits parent's mask)
    sigprocmask(SIG_SETMASK, &setSig, NULL);    //just change mask

    printf("<Key Manager> Setting a timer of %i sec\n", TIME_ALARM);

    if(signal(SIGALRM, signalsHandler) == SIG_ERR)             
        printf("\nProblem setting Sigalarm Handler\n");

    alarm(TIME_ALARM);

    while(1)            
        pause();                                //wait unitl a signal come, suspend thread
}

void serverFun(struct Request clientRequest, unsigned int usedKeys[])
{
    int byteRead = -1;
    unsigned int key = 0;

    do
    {
        printf("<Server> Waiting for a Request...\n");                
        byteRead = read(serverFIFO, &clientRequest, sizeof(struct Request));

        // Check the number of bytes read from the FIFO
        if(byteRead == -1) 
        {
            printf("<Server> Error reading FIFO\n");
        } 
        else if (byteRead != sizeof(struct Request) || byteRead == 0)
            printf("<Server> It looks like I did not receive a valid request\n");
        else 
        {
            pMutex(semid, 0);   
            key = updateTable(clientRequest, usedKeys, TABLE_SIZE, USED_KEY_ARRAY_SIZE);
            vMutex(semid, 0);

            sendResponse(&clientRequest, key);
        }
    }
    while (byteRead != -1);
}

void quit()
{
    /************* FIFO *************/
    closeFifos();   //both
    removeFifo();
    
    /************* SHARED MEMORY *************/ 
    freeSharedMemory(table); 
    removeSharedMemory(shmidServer);

    /************* SEMAPHORE *************/ 
    removeSemaphore(semid);   
}

void closeFifos()
{
    if (serverFIFO != 0 && close(serverFIFO) == -1)
        printf("Close failed\n");

    if (serverFIFO_extra != 0 && close(serverFIFO_extra) == -1)
        printf("Close failed\n");
}

void removeFifo()
{
    if (unlink(path2ServerFIFO) != 0)
        printf("Unlink failed\n"); 
}

unsigned int updateTable(struct Request request, unsigned int usedKeys[], int tableSize, int usedKeysSize)
{
    unsigned int key = 0;
    int i = 0;    
    
    /************* KEY GENERATION *************/
    do
    {
        if(isServiceValid(request.service))
            key = keyEncrypter(request.service);    //calls keyEncrypter and returns the key
        else
            return 0;                               //service not valid, key = 0
    }
    while(!isUniqueKey(key, usedKeys, tableSize, usedKeysSize));
    
    /************* INSERT KEY IN SHMEM TABLE *************/
    for(i = 0; i < tableSize; i++)
    {
        if(table[i].key == 0)
        {
            strcpy(table[i].user, request.id);
            table[i].key = key;
            table[i].timestamp = time(NULL);
            break;
        }  
    }

    if(i == tableSize)
    {
        printf("Not empty spaces available\n");    
    }
    else
    {        
        /************* INSERT KEY IN ARRAY OF GENERATED KEYS *************/
        for(i = 0; i < usedKeysSize; i++)
        {
            if(usedKeys[i] == 0)
            {
                usedKeys[i] = key;
                break;
            }  
        }
    }
    
    return key;
}

/*
    Returns:    true if the key is unique in the shared memory
                false if it's not
    Param:      key: key to evaluate

    This function access to the shared memory to check if the key is
    unique. It uses an already created semaphore (by the server).
*/
bool isUniqueKey(unsigned int key, unsigned int usedKeys[], int tableSize, int usedKeysSize)
{
    int i = 0;

    //check if key was already generated
    for(i = 0; i < usedKeysSize; i++)
    {
        if(usedKeys[i] == key)
            return false;
    }

    //check if key is already in shmem
    for(i = 0; i < tableSize; i++)
    {
        if(table[i].key == key)
            return false;
    }
     
    return true;
}

void sendResponse(struct Request *request, unsigned int key)
{
    struct Response response;
    int clientFIFO;
    char path2ClientFIFO [256];
    int byteWrite = 0;

    // Make the path of client's FIFO    
    sprintf(path2ClientFIFO, "%s%s", baseClientFIFO, request->id);

    printf("<Server> Opening FIFO %s...\n", path2ClientFIFO);
    clientFIFO = open(path2ClientFIFO, O_WRONLY);
    
    if (clientFIFO == -1) 
        printf("<Server> Open failed\n");

    // Prepare the response for the client
    response.key = key;

    printf("<Server> Sending a response\n");
    byteWrite = write(clientFIFO, &response, sizeof(struct Response));
        
    // Check the number of bytes written to the FIFO
    if (byteWrite == -1) 
    {
        printf("<Server> Write failed\n");
    } 
    else if (byteWrite != sizeof(struct Response) || byteWrite == 0)
    {
        printf("<Server> It looks like I can't write all the fields\n");
    }

    // Close the FIFO client
    if(close(clientFIFO) != 0)
        printf("<Server> Close failed\n");
}

bool isServiceValid(char *str)
{
    toUpperCase(str);

    if(strcmp(str, "STAMPA") == 0 || strcmp(str, "SALVA") == 0 || strcmp(str, "INVIA") == 0)
        return true;

    return false;
}

void toUpperCase(char *str)
{
    char upper[DIM_STRING + 1] = {""};
    int i = 0;

    while(str[i] != '\0')
    {
        upper[i] = toupper(str[i]);
        i++;
    }
    upper[i] = '\0';

    strcpy(str, upper);
}

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

void signalsHandler(int sig) 
{
    switch(sig)
    {
        case SIGALRM:                       //only keyManager
        {
            pMutex(semid, 0);

            printf("<Key Manager> KeyManager (%i) calls alarm\n", getpid());

            checkTableEvery30Sec(TABLE_SIZE);        

            alarm(TIME_ALARM);

            if(signal(SIGALRM, signalsHandler) == SIG_ERR)                   
                printf("\nProblem setting Sigalarm Handler\n"); 

            vMutex(semid, 0);

            break;
        }
        case SIGTERM:
        {
            if(keyManager == 0)             //keyManager
            {
        	    exit(0);   
            }
            else                            //server
            {          
                kill(keyManager, SIGTERM); 
                quit();                     // close FIFO, shared memory and semaphore.
                exit(0);
            }

            break;
        }
    }         
}

void checkTableEvery30Sec(int size)
{
    int i = 0;

    for(i = 0; i < size; i++)
    {
        //deletes the key if 5 minutes are passed
        if((table[i].timestamp < time(NULL)-300) && (table[i].key != 0))
        {
            strcpy(table[i].user, "");
            table[i].key = 0;
            table[i].timestamp = 0;
        }
    }  
}
