#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct Request
{
    char *idCode;
    char *service;
};

struct Response
{
    int key;
};

struct KeyTableEntry
{
    char *utente;
    int key;
    int time;  
};

int main(void)
{
    /*--------- PARTE COOPERAZIONE CLIENTREQ E UTENTE --------------*/
    char *idCode;   //malloc?
    char *service;  //malloc?
    
    
    printf("Buongiorno! \nInserire codice identificativo: \n");
    scanf("%256s", idCode);    //usare getchar?
    printf("\nInserire il servizio desiderato: \nStampa\nSalva\nInvia\n\n");
    scanf("%256s", service);
    //controllo se l'operazione scelta non è disponibile
    //magari mettere le operazioni disponbili in un array? string.h --> equals?
    //riempio struct Request con le informazioni che l'utente mi ha passato

    /*
        - apertura FIFOCLIENT per la ricezione
        - passo struct Request su FIFOSERVER
        - invio a server in FIFO che sta in attesa della request
        (con funzione opportuna)
        - Mi ritorna dalla FIFOCLIENT la chiave generata dal Server
        - chiudo FIFOCLIENT appena ho ricevuto

    */    

    int chiave;     //da togliere
    printf("\nChiave rilasciata dal Server: %d", chiave);    //lo prende dalla struct di ritorno dalla fifo
    /*--------- FINE COOPERAZIONE CLIENTREQ E UTENTE --------------*/

        //INFRAMEZZO PUBBLICITARIO


    /*--------- PARTE SERVER --------------*/
    
    /*
        Server:
        - apre la FIFOSERVER per la ricezione
        - genera la chiave --> (chiama funzione Hash)
        - prende mutex
        - crea porzione memoria condivisa
        - crea nuovo nodo nella lista TableKey
        - mette la chiave, id e time nel nodo TableKeyEntry 
        - collega il nodo alla struct TableKey (forse serve una testa a tutta la lista?)
        - libero mutex
        - creo figlio keymanager
        - metto la chiave generata nella struct Response
        - mando la struct al client
        - chiudo la FIFOSERVER appena ho mandato
        - prendo mutex
        - manda ogni 30 secondi un sig alarm 
        - se ricevo SIGTERM allora
        - prendo mutex
        - chiudo segmento memoria condiviso
        - lascio mutex
    */

    /*--------- FINE SERVER --------------*/

        //INFRAMEZZO PUBBLICITARIO




    /*--------- PARTE KEYMANAGER --------------*/
        /*
            - prende il mutex
            - ogni 30 sec controllo se ci sono chiavi + vecchie di 5 min (signal alarm?)
            - se ci sono, le rimuovo
            - se non ci sono non faccio nulla
            - lascio mutex
            - termina alla ricezione di SIGTERM da parte del server

        */

    
    
    
    /*--------- FINE CKEYMANAGER --------------*/

    return 0;
}

