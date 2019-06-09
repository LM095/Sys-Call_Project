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

struct message
{
    long mType;
    char mText[MAX_READ + 1]; /* array of chars as message body */
};

int main (int argc, char *argv[]) 
{
    struct message m = {0};
    int msgId = 0;
    size_t mSize = 0;

    printf("Hi, I'm Invia Receiver program!\n");

    msgId = msgget(30, IPC_CREAT | S_IRUSR | S_IWUSR);
    mSize = sizeof(struct message) - sizeof(long);

    if(msgrcv(msgId, &m, mSize, 1, 0) == -1)
        printf("\nError receiving message in queue");  
    
    printf("\nMessage: %s", m.mText);  

    if(msgctl(msgId, IPC_RMID,  NULL) == -1)
        printf("\nError closing queue");
    

    return 0;
}
