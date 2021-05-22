#define _POSIX_C_SOURCE 200112L 
#include "utilities.h"

//funzione che rimuove il socket
void rimozione(){
    if (unlink(SOCKNAME) < 0){
        perror("unlink");
        _exit(EXIT_FAILURE);
    }
    _exit(EXIT_SUCCESS);
}
//funzione per i segnali inviati al server, chiamata una volta ricevuto uno dei segnali da catturare
void handle_signal_action(int sig_number){
    printf("Rimozione socket.\n");
    fflush(stdout);
    //chiusura socket
    rimozione(); 
}
//funzione per controllo caratteri white-space
int controllo(char *x, int inizio, int fine){
    char carattere = *(x + inizio);
    if (inizio > fine){
        return 0;
    } 
    if((carattere == ' ')||(carattere == '\f')||(carattere == '\t')||(carattere == '\r')||(carattere == '\v')){
        return 1;
    }
    return controllo(x, ++inizio, fine);
}
//funzione che inverte i MAIUSCOLO e minuscolo
void Inversa(char *x, int inizio, int fine){ 
    char carattere = *(x + inizio);
    if(inizio > fine ){
        return;
    }
    if(isupper(carattere)){
        *(x + inizio) = tolower(carattere);
    }else{
        if(islower(carattere)){
            *(x + inizio) = toupper(carattere);
        }
    }
    Inversa(x, ++inizio, fine);
}
//thread che si occupa di iteragire con un client
void *thread_gestisci_client(void *arg){
    long connfd = (long)arg;
    //CICLO CHE VIENE CHIUSO ALLA DISCONNESSIONE COL CLIENT
    //1.   Riceve messaggi dal client
    //2.   Controlla se la stringa ricevuta contiene caratteri white-space.
    //2.1. In caso fossero presenti stampa messaggio di errore
    //3.   Se non ci fossero caratteri white-space inverte maiuscolo e minuscolo restituendolo al client.
    do{
        char buffer[BUFSIZE];
        memset(buffer, 0x0, BUFSIZE);
        int n;
        SYSCALL(n, read(connfd, buffer, BUFSIZE), "read");
        if (n == 0){
            break;
        }
        if(controllo(buffer, 0, n - 2)){
            SYSCALL(n, writen(connfd, "ERROE: HAI INSERITO UN CARATTERE NON SUPPORTATO" , 51), "write");
        } else{
            Inversa(buffer, 0, n - 2);
            SYSCALL(n, writen(connfd, buffer, sizeof(buffer)), "write");
        }

    } while (1);
    close(connfd);
    return NULL;
}
//funzione che genera un nuovo thread per ogni client che si connette al server
void creazioneT(int n_fd){
    pthread_attr_t tid;
    pthread_t thid;

    //inizializzazione attributo
    if (pthread_attr_init(&tid) != 0){
        perror("pthread_attr_init");
        close(n_fd);
        return;
    }
    // settiamo il thread in modalità detached
    if (pthread_attr_setdetachstate(&tid, PTHREAD_CREATE_DETACHED) != 0){
        perror("pthread_attr_setdetachstate");
        //liberare risorse usate per tid(attributo)
        pthread_attr_destroy(&tid);
        close(n_fd);
        return;
    }
                                                            
    if (pthread_create(&thid, &tid, thread_gestisci_client, (void *)(intptr_t)n_fd) != 0){
        perror("pthread_create");
        //liberare risorse usate per l'attributo
        pthread_attr_destroy(&tid);
        close(n_fd);
        return;
    }
}
int main(void){
    //richiamo funzione di clean al termine del programma
    if (atexit(rimozione) < 0){
        perror("atexit");
        exit(EXIT_FAILURE);
    }

    struct sigaction signal_action;
    memset(&signal_action, 0, sizeof(signal_action));
    signal_action.sa_handler = handle_signal_action;
    sigset_t handlermask;
    sigemptyset(&handlermask);
    
    //segnali per l'handler
    sigaddset(&handlermask, SIGINT);
    sigaddset(&handlermask, SIGQUIT);
    sigaddset(&handlermask, SIGTERM);
    sigaddset(&handlermask, SIGHUP);
    sigaddset(&handlermask, SIGPIPE);
    signal_action.sa_mask = handlermask;

    //gestione chiamate sigaction alla ricezione di uno dei segnali
    if (sigaction(SIGINT, &signal_action, 0) != 0){
        perror("sigaction()");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGQUIT, &signal_action, 0) != 0){
        perror("sigaction()");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGTERM, &signal_action, 0) != 0){
        perror("sigaction()");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGHUP, &signal_action, 0) != 0){
        perror("sigaction()");
        exit(EXIT_FAILURE);
    }
     if (sigaction(SIGPIPE, &signal_action, 0) != 0){
        perror("sigaction()");
        exit(EXIT_FAILURE);
    }

    int listenfd;
    SYSCALL(listenfd, socket(AF_UNIX, SOCK_STREAM, 0), "socket");
    // inizializzo indirizzo
    struct sockaddr_un serv_addr;
    memset(&serv_addr, '0', sizeof(serv_addr));
    serv_addr.sun_family = AF_UNIX;
    strncpy(serv_addr.sun_path, SOCKNAME, strlen(SOCKNAME) + 1);
    int dummy_int;

    // passo indirizzo al socket
    SYSCALL(dummy_int, bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)), "bind");
    SYSCALL(dummy_int, listen(listenfd, 1), "listen");

    int n_fd;
    //ciclo infinito che attende la connessione di un client
    do{
        SYSCALL(n_fd, accept(listenfd, (struct sockaddr *)NULL, NULL), "accept");
        creazioneT(n_fd);
    } while (1);

    close(listenfd);
    return 0;
}