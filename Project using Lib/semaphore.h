#include <stdio.h>
#include <sys/sem.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifndef SEMAPHORE_H
#define SEMAPHORE_H
#define SEM_KEY 20

union semun 
{
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

void removeSemaphore(int semid);
void vMutex (int semid, unsigned short sem_num);
void pMutex (int semid, unsigned short sem_num); 
int createSemSet(key_t semkey);
int getSemSet(key_t semkey);

#endif