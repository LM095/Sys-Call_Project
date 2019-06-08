#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <ctype.h>		//lib per funzione upper()
#include <unistd.h>
#include <fcntl.h>
#include <limits.h>
#include <time.h>
#include <signal.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <sys/stat.h>

#define DIM_STRING 255 //dimensione massima della stringa--> scelta progettuale 
#define STAMPA_MASK 1
#define SALVA_MASK 2
#define INVIA_MASK 3
#define SHM_KEY 10
#define SEM_KEY 20
#define TABLE_SIZE 1024

struct Request
{
	char id[DIM_STRING + 1];
    unsigned int key;
	char service[DIM_STRING + 1];
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

/////////////////// VARIABILI GLOBALI ///////////////////
int semid;
int shmidClientExec;
pid_t keyManager;
struct keyTable *table;
//////////////////////////////////////////////////////////

///////////////////DEFINIZIONE FUNZIONI ////////////////
int keyDecrypter(unsigned int key);
int create_sem_set(key_t semkey);
void semOp (int semid, unsigned short sem_num, short sem_op);
int alloc_shared_memory(key_t shmKey, size_t size);
void free_shared_memory(void *ptr_sh); 
void remove_semaphore(int semid);
bool isValidKey(char id[], unsigned int key, int size);
void free_shared_memory(void *ptr_sh); 
void signalsHandler(int sig); 
void *get_shared_memory(int shmid, int shmflg);

int main(int argc, char *argv[])
{
    char id[DIM_STRING + 1] = {0};  //me l'ha detto sumo
    unsigned int key = 0;
    int service = 0;
    
    if(argc < 4)
    {
        printf("Expected: user_id, key, at least one argument for the service\n");
        return 0;
    }       
    
    // create a semaphore 
    printf("<ClientExec> creating a semaphore...\n");

    semid = create_sem_set(SEM_KEY);
    semOp(semid, 0, 1);

    /////////// SHARED MEMORY //////////////
    size_t size = sizeof(struct keyTable) * TABLE_SIZE;

    //allocate a shared memory segment
    printf("<Server> allocating a shared memory segment...\n");

    shmidClientExec = alloc_shared_memory(SHM_KEY, size);
    table = (struct keyTable*)get_shared_memory(shmidClientExec, 0);

    if(signal(SIGTERM, signalsHandler) == SIG_ERR) //catch sigterm to terminate gracefully and deatach the shmem/sem
        printf("\nProblema\n");    
 
    /////////// PARAM ///////////
    strcpy(id, argv[1]);
    key = strtoul(argv[2], NULL, 10);   //lo converte in long ma noi abbiamo soli int (vedere se va bene)! da verificare se 10 va bene (è la base in teoria) 

    printf("\nchiave prima della trasformazione: %s\n", argv[2]);
    printf("\nchiave dopo la trasformazione: %u\n\n", key);

    ////////// CHECK IN MEMORIA ////////////
    
    if(!isValidKey(id, key, TABLE_SIZE))
    {
        printf("Invalid key for the requested service\n");
        free_shared_memory(table);      //mi stacco dalla sh mem
        return 0;
    }
    else
    {       
        free_shared_memory(table);      //mi stacco dalla sh mem
        service = keyDecrypter(key);
        argv += 3;  //exclude the program name, user, key

        // procedura per servizio richiesto
        switch(service)
        {
            case STAMPA_MASK:
            {          
                printf("ESEGUO LA STAMPA:\n");      
                execv("stampa", argv);
                break;
            }
            case SALVA_MASK:
            {
                printf("ESEGUO IL SALVATAGGIO:\n"); 
                execv("salva", argv);
                break;
            }
            case INVIA_MASK:
            {
                printf("ESEGUO L'INVIO:\n"); 
                execv("invia", argv);
                break;
            }
            default:
            {
                printf("\nInvalid service\n");
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
    
    //P(mutex)
    semOp(semid, 0, -1);

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

    //V(mutex)
    semOp(semid, 0, 1);
    
    return result;
}

//codice id utente, chiave, lista servizio richiesto
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
        printf("semget failed\n");

    // Initialize the semaphore set
    union semun arg;
    arg.val = 0;

    if (semctl(semid, 0, SETVAL, arg) == -1)
        printf("semctl SETALL failed\n");

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
        printf("semop failed\n");
}

void remove_semaphore(int semid)
{
    if(semctl(semid, 0, IPC_RMID, NULL) == -1)
        printf("Error closing semaphore\n");
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
        printf("shmget failed\n");

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
        printf("shmat failed\n");

    return ptr_sh;
}

void free_shared_memory(void *ptr_sh) 
{
    // detach the shared memory segments
    if (shmdt(ptr_sh) == -1)
        printf("shmdt failed\n");
}

void quit() //FARE LE FUNZIONI CON IL CONTROLLO INCORPORATO
{    
    //////// SHARED MEMORY AND SEMAPHORE /////////

    free_shared_memory(table); 
    remove_semaphore(semid);   
}

void signalsHandler(int sig) 
{
    switch(sig)
    {
        case SIGTERM:
        {   
            quit(); // close shared memory and semaphore.
            exit(0);
            break;
        }
        default:
        {
            break;
        }
    }         
}