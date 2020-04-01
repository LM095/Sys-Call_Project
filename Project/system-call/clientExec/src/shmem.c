#include "shmem.h"

/*
    Get or create a shared memory segment

    param 1: key sh mem
    param 2: specify size to allocate. If we use shmget to get an existing mem segment, 
             then the size shouldn't be bigger than the already allocated mem
*/
int allocSharedMemory(key_t shmKey, size_t size) 
{
    int shmid = shmget(shmKey, size, IPC_CREAT | S_IRUSR | S_IWUSR); //flags: if the mem is already existing, it performes a check.

    if (shmid == -1)
        printf("Failed allocating shmem\n");

    return shmid;
}

/*
    attach the shared memory

    param 1: key sh mem
    param 2: flags
*/
void *getSharedMemory(int shmid, int shmflg) 
{ 
    void *ptrsh = shmat(shmid, NULL, shmflg);   //param 2: if NULL, the kernel decides where to put the pointer

    if (ptrsh == (void *)-1)
        printf("Failed getting shmem\n");

    return ptrsh;
}

void freeSharedMemory(void *ptr_sh) 
{
    //detach the shared memory segments
    if (shmdt(ptr_sh) == -1)
        printf("Failed deataching shmem\n");
}

void removeSharedMemory(int shmid) 
{
    //delete the shared memory segment
    if (shmctl(shmid, IPC_RMID, NULL) == -1)
        printf("Failed removing shmem\n");
}