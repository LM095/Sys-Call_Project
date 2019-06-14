#include <stdio.h>
#include <sys/shm.h>
#include <stdlib.h>
#include <sys/stat.h>

#ifndef SHMEM_H
#define SHMEM_H

#define DIM_STRING 255
#define SHM_KEY 10

struct keyTable
{
    char user[DIM_STRING + 1];
    unsigned int key;
    time_t timestamp;
};

int allocSharedMemory(key_t shmKey, size_t size);
void *getSharedMemory(int shmid, int shmflg);
void freeSharedMemory(void *ptr_sh);
void removeSharedMemory(int shmid);  

#endif