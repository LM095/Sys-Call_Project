#ifndef SHMEM_H
#define SHMEM_H

#include <sys/shm.h>
#include <stdlib.h>
#include <sys/stat.h>

#define DIM_STRING 255

struct keyTable
{
    char user[DIM_STRING + 1];
    unsigned int key;
    time_t timestamp;
};

void free_shared_memory(void *ptr_sh);
void remove_shared_memory(int shmid); 
void *get_shared_memory(int shmid, int shmflg); 
int alloc_shared_memory(key_t shmKey, size_t size); 

#endif