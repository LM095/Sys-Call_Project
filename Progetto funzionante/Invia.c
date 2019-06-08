#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>

#define MAX_READ 1024
#define MSG_KEY 30

struct mymsg 
{
    long mtype;
    char mtext[MAX_READ]; /* array of chars as message body */
};

int main (int argc, char *argv[]) 
{
    struct  mymsg m;
    int msgId;
    printf("Hi, I'm Invia program!\n");

    msgId = mssget(MSG_KEY, IPC_CREAT | S_IRUSR | S_IWUSR);
    m.mtype = 1;
    
    

    return 0;
}
