/*
Realizzare un’applicazione Client-Server basata su memoria condivisa per leggere i primi 100 caratteri di
un file presente sul filesystem. Il processo Server istanza un segmento di memoria condivisa SH1 grande
a sufficienza per contenere la seguente struttura:

struct Request {
 char pathname [250]; // path di un file nel filesystem
 key_t shmkey; // shmkey di un segmento di memoria condivisa SH2
}

Appena un’istanza di Request e’ depositata in SH1, il programma Server esegue le seguenti operazioni:
a) apre in lettura il file indicato in pathname;
b.1) deposita in SH2 i primi 100 caratteri del file aperto;
b.2) se il file non esiste, deposita in SH2 il valore -1;
c) rimuove il segmento SH1
d) infine, termina

Il processo Client crea un segmento di memoria condivisa SH2 grande 100 bytes. Chiede all’utente il
pathname a un file, e deposita in SH1 la struttura Request. Appena i primi 100 bytes del file richiesto
sono depositati in SH2, il programma Client esegue le seguenti operazioni:
a) stampa a video il contenuto di SH2
b) rimuove il segmento SH2
c) rimuove i semafori utilizzati per la sincronizzazione con il Server
c) infine, termina
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/shm.h>
#include <sys/sem.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <stdbool.h>

#define DIM_STRING 255
#define SHM_KEY 10
#define SEM_KEY 20
#define REQUEST 0

/*
    Just use the key to know if that row in the table is valis:
    0: empty and/or usable
    >0: valid key
*/
struct Prova
{
    char name[DIM_STRING + 1];
    unsigned int key;
    time_t timestamp;
};

// the Request structure defines a request sent by a client
// sta dentro la libreria shared_memory.h/c


union semun {
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

///////////// FUNCTION DECLARATION ///////////// 
int create_sem_set(key_t semkey);
// stanno dentro la libreria shared_memory.h/c
int alloc_shared_memory(key_t shmKey, size_t size);
void *get_shared_memory(int shmid, int shmflg);
void free_shared_memory(void *ptr_sh);
void remove_shared_memory(int shmid);
void semOp (int semid, unsigned short sem_num, short sem_op); 

int main(void)
{
    //allocate a shared memory segment
    printf("<Server> allocating a shared memory segment...\n");
    int shmidServer = alloc_shared_memory(SHM_KEY, sizeof(struct Prova));
    
    // attach the shared memory segment, ho ottenuto il puntantore alla zona di memoria condivisa
    printf("<Server> attaching the shared memory segment...\n");
    struct Prova *p = (struct Prova*)get_shared_memory(shmidServer, 0);
    strcpy(p->name, "User1");
    p->key = 12345;
    printf("User: %s\n",p->name);
    printf("La chiave inserita è: %i\n\n",p->key);

    // create a semaphore set
    printf("<Server> creating a semaphore set...\n");
    int semid = create_sem_set(SEM_KEY);

    //lascio il controllo del semaforo, ovvero lo setta -1
    printf("<Server> inizialize the semaphore...\n");
    semOp(semid, 0, -1);

    printf("%i\n\n",p->key);
    //il server ha già la request del client, tramite fifo, quindi non deve leggere
    //da mem condivisa!
    return 0;
}

///////////// FUNCTION IMPLEMENTATIONS ///////////// 
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
