#include <stdlib.h>
#include <stdio.h>

int main (int argc, char *argv[]) 
{
    printf("Hi, I'm Stampa program!\n\n");
    int i = 1;  //to not printf the program name
    
    while(argv[i])
    {
        printf("%s ", argv[i]);
        i++;
    }

    printf("\n");

    return 0;
}