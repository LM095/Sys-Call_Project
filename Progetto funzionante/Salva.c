#include <stdlib.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

#define MAX_READ 1024
#define TXT_EXT 5

int main (int argc, char *argv[]) 
{
    printf("Hi, I'm Salva program!\n");

    char buffer[MAX_READ + 1] = {0};   
    char nameFile[MAX_READ + TXT_EXT] = {0};    
    int i = 0;
    int j = 0;
    int bufferIndex = 0;
    int destination = 0;    

    strncpy(nameFile, argv[0], strlen(argv[0]));
    strncat(nameFile, ".txt", TXT_EXT);

    for(i = 0; (i < argc) && (j <= MAX_READ); i++)
    {
        for(j = 0; j < (int)strlen(argv[i]) && (bufferIndex < MAX_READ); j++)
        {
            buffer[bufferIndex] = argv[i][j];
            bufferIndex++;
        }

        if(bufferIndex < MAX_READ)
        {
            buffer[bufferIndex] = ' ';
            bufferIndex++;
        }  
    }
    buffer[MAX_READ] = '\0';    //just to be safe, the string is always ended
    
    destination = open(nameFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    write(destination, buffer, strlen(buffer));
        
    return 0;
}
