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


union semun 
{
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
    printf("<Client> allocating a shared memory segment...\n");
    int shmidClient = alloc_shared_memory(SHM_KEY, sizeof(struct Prova));
    
    // attach the shared memory segment, ho ottenuto il puntantore alla zona di memoria condivisa
    printf("<Client> attaching the shared memory segment...\n");
    struct Prova *p2 = (struct Prova*)get_shared_memory(shmidClient, 0);
    printf("%i\n",p2->key);
    p2->key = 0;
   
    printf("%s\n", p2->name);
    printf("%i\n",p2->key);

    printf("<Client> creating a semaphore set...\n");
    int semid = create_sem_set(SEM_KEY);

    //lascio il controllo del semaforo, ovvero gli aggiungo 1 (torna a 0)
    printf("<Client> inizialize the semaphore...\n");
    semOp(semid, 0, 1);
    //il server ha già la request del client, tramite fifo, quindi non deve leggere
    //da mem condivisa!
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
