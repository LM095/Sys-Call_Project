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

    char buffer[MAX_READ] = " ";    
    char nameFile[MAX_READ + TXT_EXT] = "  ";    //+5 = .txt\0
    int i = 0, j = 0, w = 0;
    int destinazione = 0;
    
    strncpy(nameFile, argv[0], strlen(argv[0]));

    for(i = 0; i < argc; i++)
    {
        for(j = 0; j < (int)strlen(argv[i]); j++)
        {
            buffer[w] = argv[i][j];
            w++;
        }
        buffer[w] = ' ';
        w++;
    }
    buffer[w]= '\0';

    strncat(nameFile, ".txt", TXT_EXT);

    destinazione = open(nameFile, O_WRONLY | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    write(destinazione, buffer, strlen(buffer));
        
    return 0;
}
