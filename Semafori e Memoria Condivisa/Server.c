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
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <signal.h>

#define DIM_STRING 255 //dimensione massima della stringa--> scelta progettuale 
#define STAMPA_MASK 1
#define SALVA_MASK 2
#define INVIA_MASK 3
#define SHM_KEY 10
#define SEM_KEY 20
#define TABLE_SIZE 1024
#define TIME_ALARM 30

///////// STRUCT /////////////////////////
struct Request
{
	char id[DIM_STRING + 1];
	char service[DIM_STRING + 1];
};

struct Response
{
	unsigned int key;
};

struct keyTable
{
    char user[DIM_STRING + 1];
    unsigned int key;
    time_t timestamp;
};

union semun 
{
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};


///////// FUNCTIONS DEFINITIONS /////////

void quit(); 
unsigned int updateTable(struct Request request, int semid, int size);
bool isUniqueKey(unsigned int key, int size);
void sendResponse(struct Request *request, unsigned int key);
bool isServiceValid(char *str);
void toUpperCase(char *str);
unsigned int keyEncrypter(char *service);
int keyDecrypter(unsigned int key);
int create_sem_set(key_t semkey); 
int create_sem_set(key_t semkey); 
void semOp (int semid, unsigned short sem_num, short sem_op); 
void free_shared_memory(void *ptr_sh);
void remove_shared_memory(int shmid); 
void *get_shared_memory(int shmid, int shmflg); 
int alloc_shared_memory(key_t shmKey, size_t size); 
void signalsHandlerKeyManager(int sig); 
void signalsHandlerServer(int sig); 
void checkTableEvery30Sec(int size);

//////////// GLOBAL VARIABLES ///////////

char *path2ServerFIFO = "FIFOSERVER";
char *baseClientFIFO = "FIFOCLIENT.";
int semid;
int shmidServer;
pid_t keyManager;
struct keyTable *table;

// the file descriptor entry for the FIFO
int serverFIFO, serverFIFO_extra;

int main (void) 
{
    struct Request clientRequest;
    int byteRead = -1;
    unsigned int key = 0;
    sigset_t setSig; 

    printf("<Server> Making FIFO...\n");
    // FIFO with the following permissions:
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
    
    /////////////////////// INZIO SINCRONIZZAZIONE ////////////////////////////////////////

    // -------- SET SIGNAL MASK --------------
    //tutta la maschera a 1
    sigfillset(&setSig);

    //tolgo il segnale SIGTERM
    sigdelset(&setSig, SIGTERM);    //non blocca SOLO sigterm

    //sblocco solo sigterm
    sigprocmask(SIG_SETMASK, &setSig, NULL);

    //setto handler in caso di sigterm (ricorda che prima dobbiamo aprire fifo, memoria e semafori, altrimenti alla chiusura non si chiude tutto bene)
    if(signal(SIGTERM, signalsHandler) == SIG_ERR) 
        printf("\nProblema");


    // create a semaphore 
    printf("<Server> creating a semaphore...\n");
    semid = create_sem_set(SEM_KEY);

    //setto il semaforo a 0
    printf("<Server> inizialize the semaphore...\n");
    semOp(semid, 0, 0);

    //-----CREAZIONE MEM CONDIVISA -----
    size_t size = sizeof(struct keyTable) * TABLE_SIZE;

    //allocate a shared memory segment
    printf("<Server> allocating a shared memory segment...\n");
    shmidServer = alloc_shared_memory(SHM_KEY, size);
    
    // nel main non serve!!!!
    // attach the shared memory segment, ho ottenuto il puntantore alla zona di memoria condivisa
    //printf("<Server> attaching the shared memory segment...\n");    
    table = (struct keyTable*)get_shared_memory(shmidServer, 0);

    //rilascio il semaforo +1 V(MUTEX) (serve???)
    semOp(semid, 0, 1);
    
    //----- CREAZIONE KEYMANAGER ----
    keyManager = fork();        //keyManager è globale

    switch(keyManager)
    {
        case -1:
        {
            printf("ERRORE FORK() !\n");
            break;
        }
        case 0:     //figlio
        {
            sigdelset(&setSig, SIGALRM);    //non blocca sigalarm

            /*The last argument, oldset, is used to return information about the old process signal mask. 
            If you just want to change the mask without looking at it, pass a null pointer as the oldset argument. */
            sigprocmask(SIG_SETMASK, &setSig, NULL); 

            printf("\n - Imposto un timer di %i secondi finchè non arriva sigterm\n", TIME_ALARM);

            if(signal(SIGALRM, signalsHandler) == SIG_ERR) 
                printf("\nProblema");

            alarm(TIME_ALARM);

            while(1)            
                pause(); //wait unitl a signal come, suspend thread

            break;
        }
        default:    //padre
        {
            do
			{
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
                {
                    key = updateTable(clientRequest, semid, TABLE_SIZE);
                    sendResponse(&clientRequest, key);
                }
            }
            while (byteRead != -1);
            break;
        }
    }    
    
    return 0;
}

//////////// FUNCTIONS IMPLEMENTATIONS ///////////////////

void quit() //FARE LE FUNZIONI CON IL CONTROLLO INCORPORATO
{
    //////// FIFO ////////
    // Close the FIFO
    if (serverFIFO != 0 && close(serverFIFO) == -1)
        printf("close failed");

    if (serverFIFO_extra != 0 && close(serverFIFO_extra) == -1)
        printf("close failed");

    // Remove the FIFO
    if (unlink(path2ServerFIFO) != 0)
        printf("unlink failed");
    
    //////// SHARED MEMORY /////////

    free_shared_memory(table); 
    remove_shared_memory(shmidServer);
    remove_semaphore(semid);
   
}

unsigned int updateTable(struct Request request, int semid, int size)
{
    unsigned int key = 0;
    int i = 0;
    // forse qui o forse in isuniquekey
    //-1 sem
    //prendo il semaforo
     //cerco di prendere il semaforo con -1

    //P(MUTEX)
    semOp(semid, 0, -1);   
    
    // ---- CREAZIONE CHIAVE -----
    do
    {
        if(isServiceValid(request.service))
            key = keyEncrypter(request.service);  //calls keyEncrypter and returns the key
        else
            return 0;   //service not valid, key = 0
    }
    while(!isUniqueKey(key,size)); //funzione che checkka in memoria condivisa e torna un bool per vedere se la chiave esiste

    //qua siamo sicuri che la chiave è univoca
    //la inserisco nella tabella con timestamp e id

    for(i = 0; i < size; i++)
    {
        if(table[i].key == 0)
        {
            strcpy(table[i].user, request.id);
            table[i].key = key;
            table[i].timestamp = time(NULL);
            break;
        }  
        else
        {
            printf("MEMORIA ESAURITA\n");
        }
    }

    //mollo semaforo +1
    //V(MUTEX)
    semOp(semid, 0, 1);

    return key;
}

/*
    Returns:    true if the key is unique in the shared memory
                false if it's not
    Param:      key: key to evaluate

    This function access to the shared memory to check if the key is
    unique. It uses an already created semaphore (by the server).
*/
bool isUniqueKey(unsigned int key, int size)
{
    unsigned int i = 0;
    //scorro tutta la memoria condivisa
    for(i = 0; i < size; i++)
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

    // make the path of client's FIFO    
    sprintf(path2ClientFIFO, "%s%s", baseClientFIFO, request->id);

    printf("<Server> opening FIFO %s...\n", path2ClientFIFO);
    // Open the client's FIFO in write-only mode
    clientFIFO = open(path2ClientFIFO, O_WRONLY);  //sola scrittura
    
    if (clientFIFO == -1) 
    {
        printf("<Server> open failed");
        return;
    }

    // Prepare the response for the client
    response.key = key;

    printf("<Server> sending a response\n");
    // Write the Response into the opened FIFO
    byteWrite = write(clientFIFO, &response, sizeof(struct Response));
        
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

    // Close the FIFO client
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

int create_sem_set(key_t semkey) 
{
    // Create a semaphore set with 1 semaphore
    int semid = semget(semkey, 1, IPC_CREAT | S_IRUSR | S_IWUSR);   //IPC_CREAT: Create entry if key does not exist
                                                                    //S_IRUSR: read for owner
                                                                    //S_IWUSR: write for owner
    if (semid == -1)
        printf("semget failed");

    // Initialize the semaphore set
    union semun arg;
    arg.val = 0;

    if (semctl(semid, 0, SETVAL, arg) == -1)
        printf("semctl SETALL failed");

    return semid;
}

void semOp (int semid, unsigned short sem_num, short sem_op) 
{
    //struct sembuf sop = {.sem_num = sem_num, .sem_op = sem_op, .sem_flg = 0}; //assegnamento
    struct sembuf sop;

    sop.sem_num = sem_num;  //passa; numero del semaforo dentro array dei semafori
    sop.sem_op = sem_op;    //passa; operation to be performed
    sop.sem_flg = 0;        //  


    if (semop(semid, &sop, 1) == -1)
        printf("semop failed");
}

// sta dentro la libreria shared_memory.h/c
int alloc_shared_memory(key_t shmKey, size_t size) 
{
    /*
        get, or create, a shared memory segment

        param1: key della mem condivsa
        param2: specifica la size che dobbiamo allocare.
                se usiamo shmget per allocare memoria già esistente (quindi una get)
                allora questa non deve superare le dimensione della mem già allocata
        param3: flag per la creazione della mem
                se la mem esiste già, si usano per un check sulla mem condivisa
    */
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR);
    if (shmid == -1)
        printf("shmget failed");

    return shmid;
}

void *get_shared_memory(int shmid, int shmflg) 
{ 
    /*
        attach the shared memory
        torna il puntatore all'ind di mem al quale la mem condivisa si è attaccata
        o -1 se ci sono errori
    
        param 1: key della mem condivisa (sempre quella)
        param 2: NULL: sceglie il kernel dove mettere il nuovo pezzo. Questo param e il
                       successivo sono ignorati
                 diverso da NULL: lo diciamo noi (attenzione, meno portabile!)
        param 3: flag che specificano i permessi, ma noi qua non abbiamo flag 
                 qua è 0, perchè tanto il precedente è NULL e viene ignorato
    */
    void *ptr_sh = shmat(shmid, NULL, shmflg);
    if (ptr_sh == (void *)-1)
        printf("shmat failed");

    return ptr_sh;
}

void free_shared_memory(void *ptr_sh) 
{
    // detach the shared memory segments
    if (shmdt(ptr_sh) == -1)
        printf("shmdt failed");
}

void remove_shared_memory(int shmid) 
{
    // delete the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        printf("shmctl failed");
}

void signalsHandlerKeyManager(int sig) 
{
    switch(sig)
    {
        case SIGALRM:   //è solo il figlio
        {
			//P(MUTEX)  ***prendiamo il semaforo!!! importante!!!
    		semOp(semid, 0, -1);
            printf("Pid che chiama sig alarm (deve essere quello del figlio!!): %i\n", getpid()); //deve essere quello del figlio!!!

            checkTableEvery30Sec(TABLE_SIZE);

            alarm(TIME_ALARM);

            if(signal(SIGALRM, signalsHandler) == SIG_ERR) 
                printf("\nProblema");

			//V(MUTEX)
    		semOp(semid, 0, 1);
            break;
        }
        case SIGTERM:
        {
        	exit(0);                    
            break;
        }
    }         
}

void checkTableEvery30Sec(int size)
{
    int i = 0;

    for(i = 0; i < TABLE_SIZE; i++)
    {
        //elimino la chiave se sono passati 5 minuti
        if((table[i].timestamp < time(NULL)-300) && (table[i].key != 0))
        {
            strcpy(table[i].user, "");
            table[i].key = 0;
            table[i].timestamp = 0;
        }
    }  
}


void signalsHandlerServer(int sig) 
{
    switch(sig)
    {
        case SIGTERM:
        {
            printf("Sono %i e sto per killare qualcosa\n", getpid());
          
			kill(keyManager, SIGTERM);    //figlio ko
			quit(); // close FIFO, shared memory and semaphore.
			exit(0);
			
            break;
        }
    }        
}