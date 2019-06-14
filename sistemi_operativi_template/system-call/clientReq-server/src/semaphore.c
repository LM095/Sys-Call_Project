#include "semaphore.h"

void removeSemaphore(int semid)
{
    if(semctl(semid, 0, IPC_RMID, NULL) == -1)
        printf("Error closing semaphore\n");
}

void vMutex (int semid, unsigned short sem_num) 
{
    struct sembuf sop;

    sop.sem_num = sem_num;  // index sem num
    sop.sem_op = 1;         // operation to be performed
    sop.sem_flg = 0;        

    if (semop(semid, &sop, 1) == -1)
        printf("V(Mutex) failed\n");
}

void pMutex (int semid, unsigned short sem_num) 
{
    struct sembuf sop;

    sop.sem_num = sem_num;  // index sem num
    sop.sem_op = -1;        // operation to be performed
    sop.sem_flg = 0;        

    if (semop(semid, &sop, 1) == -1)
        printf("P(Mutex) failed\n");
}

int createSemSet(key_t semkey) 
{
    union semun arg;
    // Create a semaphore set with 1 semaphore
    int semid = semget(semkey, 1, IPC_CREAT | S_IRUSR | S_IWUSR);   //IPC_CREAT: Create entry if key does not exist
                                                                    //S_IRUSR: read for owner
                                                                    //S_IWUSR: write for owner
    if (semid == -1)
        printf("Failed creating semaphored\n");

    // Initialize the semaphore set to 0

    arg.val = 0;

    if (semctl(semid, 0, SETVAL, arg) == -1)
        printf("Failed setting semaphore\n");

    return semid;
}

int getSemSet(key_t semkey)
{
    // Get a semaphore set with 1 semaphore
    int semid = semget(semkey, 1, IPC_CREAT | S_IRUSR | S_IWUSR);   //IPC_CREAT: Create entry if key does not exist
                                                                    //S_IRUSR: read for owner
                                                                    //S_IWUSR: write for owner
    if (semid == -1)
        printf("Failed getting semaphore\n");

    return semid;
}