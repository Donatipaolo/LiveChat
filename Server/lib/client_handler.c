#include "client_handler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h> 

struct client_list* list_client = NULL;

/*static void memcpy(void* buffer1, void* buffer2, size_t len){

    for(size_t i = 0; i < len; i++){
        *((unsigned char*)buffer1 + i) = *((unsigned char*)buffer2 + i);         
    }
}*/

//Function

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void create_client(int fd, char buffer[]){

    //inizializzo il pacchetto
    struct client_handler_pck pck;
    struct client cli;

    //Inizializzo i valori del pacchetto
    pck.type = 0;
    pck.len = sizeof(struct client);

    //Inizializzo il client
    cli.fd = fd;
    cli.status = 0;
    memcpy(cli.username,buffer,USERNAME_LENGHT);

    //Inserisco il client nel pacchetto
    memcpy(pck.value,&cli,sizeof(struct client));

    //invio il pacchetto
    write(client_handler_id,&pck,sizeof(struct client_handler_pck));
}

//Funzione che invia una richiesta di eliminazione di un utente dato il file descriptor con cui comunica
void remove_client(int fd){

    //inizializzo il pacchetto
    struct client_handler_pck pck;
    
    //Inizializzo le variabili
    int value = fd;

    //Inizializzo i valori del pacchetto
    pck.type = 1;
    pck.fdw = -1;
    memcpy(pck.value,&value,sizeof(int));
    pck.len = sizeof(int);

    //Invio il pacchetto
    write(client_handler_id,&pck,sizeof(struct client_handler_pck));
}

//Funzione che restituisce l'indirizzo di memoria di un client dato l'fd
struct client* get_client(int fd){

    //inizializzo il pacchetto
    struct client_handler_pck pck;
    struct client* cli;

    //Inizializzo la pipe di comunicazione
    int pipefd[2];
    pipe(pipefd);

    //Inizializzo i valori
    int value = fd;
    pck.type = 2;
    pck.fdw = pipefd[1];
    memcpy(pck.value, &value, sizeof(int));
    pck.len = sizeof(int);

    //Invio il pacchetto
    write(client_handler_id,&pck,sizeof(struct client_handler_pck));

    //leggo la risposta
    read(pipefd[0],&cli,sizeof(struct client*));

    //Chiudo la comunicazione
    close(pipefd[0]);

    return cli;
}

//Funzione che verifica se un utente è libero dato il suo fd
int free_client(int fd){

    //inizializzo il pacchetto
    struct client_handler_pck pck;

    //Inizializzo la pipe di comunicazione
    int pipefd[2];
    pipe(pipefd);

    //Inizializzo i valori
    int value = fd;
    pck.type = 4;
    pck.fdw = pipefd[1];
    memcpy(pck.value, &value, sizeof(int));
    pck.len = sizeof(int);

    //Invio il pacchetto
    write(client_handler_id,&pck,sizeof(struct client_handler_pck));

    //Leggo la risposta
    read(pipefd[0],&value,sizeof(int));

    //Chiudo la comunicazione
    close(pipefd[0]);

    //controllo il risultato
    return value;
}

//Funzione che verifica se un utente esiste dato il suo fd
int exists_client(int fd){
    //inizializzo il pacchetto
    struct client_handler_pck pck;

    //Inizializzo la pipe di comunicazione
    int pipefd[2];
    pipe(pipefd);

    //Inizializzo i valori
    int value = fd;
    pck.type = 3;
    pck.fdw = pipefd[1];
    memcpy(pck.value, &value, sizeof(int));
    pck.len = sizeof(int);

    //Invio il pacchetto
    write(client_handler_id,&pck,sizeof(struct client_handler_pck));

    //Leggo la risposta
    read(pipefd[0],&value,sizeof(int));

    //Chiudo la comunicazione
    close(pipefd[0]);

    //controllo il risultato
    return value;
}

//Chiamata al server di stampare tutti client a schermo (solo per il server)
void server_print_client(){

    //inizializzo il pacchetto
    struct client_handler_pck pck;

    pck.type = 6;

    //Invio il pacchetto
    write(client_handler_id,&pck,sizeof(struct client_handler_pck));
}

//Funzione che verifica se un username è stato preso
int is_taken_client(char buffer[USERNAME_LENGHT]){
    //inizializzo il pacchetto
    struct client_handler_pck pck;

    //Inizializzo la pipe di comunicazione
    int pipefd[2];
    pipe(pipefd);

    //Inizializzo i valori
    int value;
    pck.type = 7;
    pck.fdw = pipefd[1];
    memcpy(pck.value,buffer,USERNAME_LENGHT);
    pck.len = USERNAME_LENGHT;

    //Invio i valori
    write(client_handler_id,&pck,sizeof(struct client_handler_pck));

    //leggo i risultati
    read(pipefd[0],&value, sizeof(int));

    //Chiudo la comunicazione
    close(pipefd[0]);

    return value;
}

//Funzione che cambia l'username di un client
void change_username_client(int fd,char buffer[USERNAME_LENGHT]){
    //inizializzo il pacchetto
    struct client_handler_pck pck;

    //Inizializzo i valori
    int value = fd;
    pck.type = 8;
    memcpy(pck.value,buffer,USERNAME_LENGHT);
    memcpy(pck.value + USERNAME_LENGHT,&value,sizeof(int));
    pck.len = sizeof(struct username_pck);

    //invio il pacchetto
    write(client_handler_id,&pck,sizeof(struct client_handler_pck));
}

//Funzione che dato l'username di un client restituisce l'indirizzo della struct
struct client* get_client_by_username(char buffer[USERNAME_LENGHT]){
    //inizializzo il pacchetto
    struct client_handler_pck pck;
    struct client* cli;

    //Inizializzo la pipe di comunicazione
    int pipefd[2];
    pipe(pipefd);

    //Inizializzo i valori
    pck.type = 9;
    pck.fdw = pipefd[1];
    memcpy(pck.value, buffer, USERNAME_LENGHT);
    pck.len = USERNAME_LENGHT;

    //Invio il pacchetto
    write(client_handler_id,&pck,sizeof(struct client_handler_pck));

    //leggo la risposta
    read(pipefd[0],&cli,sizeof(struct client*));

    //Chiudo la comunicazione
    close(pipefd[0]);

    return cli;
}


void clients_update_pck(int sockfd){
    struct client_handler_pck pck;
    //Definisco  il pacchetto
    pck.type = 10;
    memcpy(pck.value,&sockfd,sizeof(int));

    write(client_handler_id,&pck,sizeof(struct client_handler_pck));
}


void change_username_server(int clientfd){
    //Inizializzazione della pipe
    int pipefd[2];
    pipe(pipefd);
    pthread_t id;

    //Inizializzazione del thread per il change name
    struct parameters *par = (struct parameters*)malloc(sizeof(struct parameters));
    par->boolean = 1;
    par->file_descriptor = clientfd;
    par->pipefdr = pipefd[0];
    
    FD_CLR(clientfd,&fd);

    //Creo il thread
    pthread_create(&id,NULL,init_client,par);

    write(pipefd[1],&id,sizeof(pthread_t));
    close(pipefd[1]);

    create_thread(id,-1,7);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void read_client(struct client_handler_pck pck,int* fd, char *buffer, int* status){
    struct client cli;
    memcpy((void*)&cli,(void*)pck.value,pck.len);

    *fd = cli.fd;
    *status = cli.status;
    memcpy((void*)buffer,(void*)cli.username,USERNAME_LENGHT);
}

struct client read_struct_client(struct client_handler_pck pck){
    struct client cli;
    memcpy((void*)&cli,(void*)pck.value,pck.len);
    return cli;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void client_push(struct client_list **head, struct client cli) {
    // Crea un nuovo nodo per la lista
    struct client_list *new_node = (struct client_list *)malloc(sizeof(struct client_list));
    if (new_node == NULL) {
        perror("Errore nell'allocazione della memoria");
        return;
    }

    // Alloca e copia i dati del client nella nuova struttura
    new_node->cli = (struct client *)malloc(sizeof(struct client));
    if (new_node->cli == NULL) {
        perror("Errore nell'allocazione della memoria per il client");
        free(new_node);
        return;
    }
    memcpy(new_node->cli, &cli, sizeof(struct client));
    
    // Inserisce il nuovo nodo all'inizio della lista
    new_node->next = *head;
    *head = new_node;
}

void client_remove(struct client_list **head, int fd) {
    struct client_list *temp = *head;
    struct client_list *prev = NULL;

    // Scorri la lista per trovare il client con il fd corrispondente
    while (temp != NULL && temp->cli->fd != fd) {
        prev = temp;
        temp = temp->next;
    }

    // Se il client non è stato trovato, ritorna
    if (temp == NULL) {
        return;
    }

    // Se il client da rimuovere è il primo nella lista
    if (prev == NULL) {
        *head = temp->next;
    } else {
        prev->next = temp->next;
    }

    // Libera la memoria del nodo
    free(temp->cli);
    free(temp);
}

struct client* client_get(struct client_list* head, int fd) {
    struct client_list *temp = head;

    // Scorri la lista fino a trovare il client con il fd corrispondente
    while (temp != NULL) {
        if (temp->cli->fd == fd) {
            return temp->cli; // restituisce il puntatore al client
        }
        temp = temp->next;
    }

    // Se non trovato, ritorna NULL
    return NULL;
}

int client_exists(struct client_list* head, int fd) {
    struct client_list* temp = head;

    // Scorre la lista per cercare un client con il fd dato
    while (temp != NULL) {
        if (temp->cli->fd == fd) {
            return 1; // Client trovato, restituisce 1
        }
        temp = temp->next; // Vai al prossimo nodo nella lista
    }

    return 0; // Client non trovato, restituisce 0
}

int client_free(struct client_list* head, int fd) {
    struct client_list* temp = head;

    // Scorre la lista per cercare il client con il fd specificato
    while (temp != NULL) {
        if (temp->cli->fd == fd) {
            // Se il client è trovato e il suo stato è 0, restituisce 1
            if (temp->cli->status == 0) {
                return 1; // Client trovato con status 0
            }
            return 0; // Il client esiste ma non ha status 0
        }
        temp = temp->next; // Passa al prossimo nodo
    }

    return 0; // Client non trovato
}

struct client* client_get_by_username(struct client_list* head,const char buffer[USERNAME_LENGHT]){
    // Creo un puntatore temporaneo per scorrere la lista
    struct client_list* current = head;

    while(current != NULL){
        if(strcmp(buffer,current->cli->username) == 0)
            return current->cli;
        current = current->next;
    }
    return NULL;
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void* client_handler(void* arg){
    //printf("Client Handler creato\n");

    int fdr = *((int*)arg);//read end of comunication pipe
    free(arg);
    //printf("Sto leggendo dalla pipe : %d\n",fdr);
    struct client_handler_pck pck;
    struct client *cli;
    struct username_pck upck;
    int value;

    //Inizio del ciclo
    while(1){
        //Leggo dalla pipe in attesa di richieste
        //printf("In attesa di lettura dalla pipe\n");
        if (read(fdr, &pck, sizeof(struct client_handler_pck)) == -1) {
            perror("Client-Handler Read");
            pthread_exit(NULL);  
        }

        switch(pck.type){

            case 0: //Inserimento di un nuovo client
                client_push(&list_client, read_struct_client(pck));
                break;
            case 1: //Rimozione di un client dato l'fd
                client_remove(&list_client,*((int*)pck.value));
                break;
            case 2: //lettura di un client
                cli = client_get(list_client,*((int*)pck.value));
                write(pck.fdw,&cli,sizeof(struct client *));
                close(pck.fdw);
                break;
            case 3: //controlla se un client esiste
                value = client_exists(list_client,*((int*)pck.value));
                write(pck.fdw,&value,sizeof(int));
                close(pck.fdw);
                break;
            case 4://controllo se il client è libero
                value = client_free(list_client,*((int*)pck.value));
                write(pck.fdw,&value,sizeof(int));
                close(pck.fdw);
                break;
            case 5: //close signal
                close(fdr);
                pthread_exit(NULL);
                break;
            case 6:
                print_client_list(list_client);
                break;
            case 7: //Controllo se esiste un client con l'username passato
                memcpy((void*)&upck,(void*)pck.value,sizeof(struct username_pck));
                if(username_taken(list_client,upck.buffer)){
                    value = 1;
                    write(pck.fdw,&value,sizeof(int));
                }
                else{
                    value = 0;
                    write(pck.fdw,&value,sizeof(int));
                }

                close(pck.fdw);
                break;
            case 8:
                memcpy((void*)&upck,(void*)pck.value,sizeof(struct username_pck));
                change_username(list_client,upck.buffer,upck.fd);
                break;
            case 9:
                cli = client_get_by_username(list_client,(char*)pck.value);
                write(pck.fdw,&cli,sizeof(struct client*));
                close(pck.fdw);
                break;
            case 10:
                send_all_username(*((int*)pck.value));
                break;
            default:
                break;
        }
    }
}


// Funzione per stampare un client
void print_client(const struct client* c) {
    if (c == NULL) return;

    printf("Client:\n");
    printf("  FD: %d\n", c->fd);
    printf("  Username: %s\n", c->username);
    printf("  Status: %d\n", c->status);
}

// Funzione per stampare tutta la lista di client
void print_client_list(const struct client_list* head) {
    const struct client_list* current = head;
    int index = 0;

    while (current != NULL) {
        printf("=== Client #%d ===\n", index++);
        print_client(current->cli);
        current = current->next;
    }
}

int username_taken(const struct client_list* head,const char buffer[USERNAME_LENGHT]){
    const struct client_list* current = head;

    while(current != NULL){
        if(strcmp(current->cli->username,buffer) == 0)
            return 1;
        current = current->next;
    }

    return 0;

}

void change_username(const struct client_list* head,const char buffer[USERNAME_LENGHT], int fd){
    const struct client_list* current = head;

    while(current != NULL){
        if(current->cli->fd == fd){
            memcpy(current->cli->username,buffer,USERNAME_LENGHT);
            return;
        }
        current = current->next;
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////
//Funzioni per clients update
void send_all_username(int sockfd){
    int dim = size(list_client) -1;//Oltre se stesso
    struct client_info info;

    write(sockfd,&dim,sizeof(int));
    if(dim == 0) return;

    struct client_list* current = list_client;

    while(current != NULL){
        if(current->cli->fd != sockfd){
            info.status= current->cli->status;
            memcpy(info.buffer,current->cli->username,USERNAME_LENGHT);
            write(sockfd,&info,sizeof(struct client_info));
        }
        current = current->next;
    }
}

int size(struct client_list* head){
    struct client_list* current = head;
    int i = 0;

    while(current != NULL){
        i++;
        current = current->next;
    }

    return i;
}
