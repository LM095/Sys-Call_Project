#ifndef SEMAPHORE_H
#define SEMAPHORE_H

#include <sys/sem.h>
#include <stdlib.h>
#include <sys/stat.h>

#define SEM_KEY 20

union semun 
{
    int val;
    struct semid_ds * buf;
    unsigned short * array;
};

int create_sem_set(key_t semkey); 
void semOp (int semid, unsigned short sem_num, short sem_op); 
void remove_semaphore(int semid);

#endif