#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <sys/msg.h>

#define MAX_READ 1024

struct message
{
    long mType;
    char mText[MAX_READ + 1]; /* array of chars as message body */
};

int main (int argc, char *argv[]) 
{
    struct message m = {0};
    int msgId = 0;
    int i = 0;
    int j = 0;
    int bufferIndex = 0;
    size_t mSize = 0;
    key_t key = 0;

    printf("Hi, I'm Invia program!\n");

    key = atoi(argv[0]);

    msgId = msgget(key, IPC_CREAT | S_IRUSR | S_IWUSR);
    m.mType = 1;

    for(i = 0; (i < argc) && (j <= MAX_READ); i++)
    {
        for(j = 0; j < (int)strlen(argv[i]) && (bufferIndex < MAX_READ); j++)
        {
            m.mText[bufferIndex] = argv[i][j];
            bufferIndex++;
        }

        if(bufferIndex < MAX_READ)
        {
            m.mText[bufferIndex] = ' ';
            bufferIndex++;
        }  
    }
    m.mText[MAX_READ] = '\0';    //just to be safe, the string is always ended

    mSize = sizeof(struct message) - sizeof(long);

    if(msgsnd(msgId, &m, mSize, 0) == -1)
        printf("\nError sending message in queue");        

    return 0;
}
