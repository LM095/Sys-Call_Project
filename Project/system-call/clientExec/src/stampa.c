#include <stdlib.h>
#include <stdio.h>

int main (int argc, char *argv[]) 
{
    printf("Hi, I'm Stampa program!\n\n");
    int i = 0; 
    
    while(argv[i])
    {
        printf("%s ", argv[i]);
        i++;
    }

    printf("\n");

    return 0;
}