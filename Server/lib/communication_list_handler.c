//CODIE DELLE API

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "communication_list_handler.h"
#include <string.h>

//List head initializer
struct communication_list * list_communication = NULL;

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//API function

//Funzione che aggiunge una entry nella communication list
void create_communication(struct client* client1,struct client* client2,pthread_t id, int fdw){
    
    //Inizializzazione delle variabili
    struct communication_list_handler_pck pck;
    
    //definisco il pacchetto
    pck.fdw = -1; // per la comuncicazione con la communication list handler
    pck.type = 0;
    pck.len = sizeof(struct communication) - sizeof(int);
    unsigned char* ptr = (unsigned char*)pck.value;
    memcpy(ptr,&client1,sizeof(struct client*)); ptr = ptr + sizeof(struct client*);
    memcpy(ptr,&client2,sizeof(struct client*)); ptr = ptr + sizeof(struct client*);
    memcpy(ptr,&id,sizeof(pthread_t)); ptr = ptr + sizeof(pthread_t);
    memcpy(ptr,&fdw,sizeof(int));

    //Invio il messaggio
    write(communication_list_handler_id,&pck,sizeof(struct communication_list_handler_pck));    
}

void remove_communication_id(int id){
    //Inizializzazione delle variabili
    struct communication_list_handler_pck  pck;

    //Definizione del pacchetto
    pck.type = 1;
    pck.fdw = -1;
    pck.len = sizeof(int);
    memcpy(pck.value,&id,sizeof(int));

    //Invio il pacchietto
    write(communication_list_handler_id,&pck,sizeof(struct communication_list_handler_pck));
}

void remove_communication_tr(pthread_t tr){
    //Inizializzazione delle variabili
    struct communication_list_handler_pck pck;

    //Definizione del pacchetto
    pck.type = 2;
    pck.fdw = -1;
    pck.len = sizeof(pthread_t);
    memcpy(pck.value,&tr,sizeof(pthread_t));

    //Invio il pacchetto
    write(communication_list_handler_id,&pck,sizeof(struct communication_list_handler_pck));
}

struct communication* get_communication_addr_id(int id){
    //Inizializzazione delle variabili
    struct communication_list_handler_pck pck;
    struct communication* comm;
    int pipefd[2];
    pipe(pipefd);

    //Definizione del pacchetto
    pck.type = 3;
    pck.fdw = pipefd[1];
    pck.len = sizeof(int);
    memcpy(pck.value,&id,sizeof(int));

    //Invio il messaggio
    write(communication_list_handler_id,&pck,sizeof(struct communication_list_handler_pck));
    
    //leggo il risultato
    read(pipefd[0],&comm,sizeof(struct communication*));
    close(pipefd[0]);

    //Return the value
    return comm;
}

struct communication* get_communication_addr_tr(pthread_t tr){
    //Inizializzazione delle variabili
    struct communication_list_handler_pck pck;
    struct communication* comm;
    int pipefd[2];
    pipe(pipefd);

    //Definizione del pacchetto
    pck.type = 4;
    pck.fdw = pipefd[1];
    pck.len = sizeof(pthread_t);
    memcpy(pck.value,&tr,sizeof(pthread_t));

    //Invio il messaggio
    write(communication_list_handler_id,&pck,sizeof(struct communication_list_handler_pck));
    
    //leggo il risultato
    read(pipefd[0],&comm,sizeof(struct communication*));
    close(pipefd[0]);

    //Return the value
    return comm;
}

pthread_t get_communication_tr(int id){
    //Inizializzazione delle variabili
    struct communication_list_handler_pck pck;
    pthread_t tr;
    int pipefd[2];
    pipe(pipefd);

    //Definizione del pacchetto
    pck.type = 5;
    pck.fdw = pipefd[1];
    pck.len = sizeof(int);
    memcpy(pck.value,&id,sizeof(int));

    //Invio il messaggio
    write(communication_list_handler_id,&pck,sizeof(struct communication_list_handler_pck));

    //Leggo il messaggio
    read(pipefd[0],&tr,sizeof(pthread_t));
    close(pipefd[0]);

    //return the value
    return tr;
}

int get_communication_id(pthread_t tr){
    //Inizializzazione delle variabili
    struct communication_list_handler_pck pck;
    int id;
    int pipefd[2];
    pipe(pipefd);

    //Definizione del pacchetto
    pck.type = 6;
    pck.fdw = pipefd[1];
    pck.len = sizeof(pthread_t);
    memcpy(pck.value,&tr,sizeof(pthread_t));

    //Invio il messaggio
    write(communication_list_handler_id,&pck,sizeof(struct communication_list_handler_pck));

    //Leggo il messaggio
    read(pipefd[0],&id,sizeof(int));
    close(pipefd[0]);

    //return the value
    return id;
}

void print_communication_list(){
    //Inizializzo il pacchetto
    struct communication_list_handler_pck pck;
    
    //Definisco il pacchetto
    pck.type = 8;
    pck.len = 0;
    pck.fdw = -1;

    //Invio il pacchetto
    write(communication_list_handler_id,&pck,sizeof(struct communication_list_handler_pck));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//List function

void communication_add(struct communication_list** head, struct client* client1, struct client* client2, pthread_t id, int fdw) {
    static int communication_id_counter = 1;

    struct communication* new_com = malloc(sizeof(struct communication));
    if (!new_com) return;

    new_com->client1 = client1;
    new_com->client2 = client2;
    new_com->communication_id = communication_id_counter++;
    new_com->id = id;
    new_com->fdw = fdw;

    struct communication_list* new_node = malloc(sizeof(struct communication_list));
    if (!new_node) {
        free(new_com);
        return;
    }

    new_node->com = new_com;
    new_node->next = *head;
    *head = new_node;
}

void communication_remove_id(struct communication_list** head, int id) {
    struct communication_list *current = *head, *prev = NULL;
    while (current != NULL) {
        if (current->com->communication_id == id) {
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->com);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

void communication_remove_tr(struct communication_list** head, pthread_t tr) {
    struct communication_list *current = *head, *prev = NULL;
    while (current != NULL) {
        if (pthread_equal(current->com->id, tr)) {
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            free(current->com);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

struct communication* addr_get_communication_id(struct communication_list** head, int id) {
    struct communication_list* current = *head;
    while (current != NULL) {
        if (current->com->communication_id == id) {
            return current->com;
        }
        current = current->next;
    }
    return NULL;
}

struct communication* addr_get_communication_tr(struct communication_list** head, pthread_t tr) {
    struct communication_list* current = *head;
    while (current != NULL) {
        if (pthread_equal(current->com->id, tr)) {
            return current->com;
        }
        current = current->next;
    }
    return NULL;
}

int communication_get_id(struct communication_list** head, pthread_t tr) {
    struct communication* com = addr_get_communication_tr(head, tr);
    if (com) {
        return com->communication_id;
    }
    return -1;
}

pthread_t communication_get_tr(struct communication_list** head, int id) {
    struct communication* com = addr_get_communication_id(head, id);
    if (com) {
        return com->id;
    }
    return (pthread_t)0; // valore "null" per pthread_t
}

void communication_print_list(struct communication_list** head) {
    struct communication_list* current = *head;
    while (current != NULL) {
        printf("Communication ID: %d, Thread ID: %lu, FDW: %d\n",
               current->com->communication_id,
               current->com->id,
               current->com->fdw);
        current = current->next;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Funzione principale
void* communication_list_handler(void* arg){

    //Inizializzazione delle variabili necessarie
    int fdr = *((int*)arg);
    free(arg);
    struct communication_list_handler_pck pck;
    
    //variabili utilizzate per le chiamate
    int id,fdw;
    pthread_t tr;
    struct communication* comm;
    struct client* client1;
    struct client* client2;
    unsigned char* ptr;

    while(1){

        //Leggo dalla pipe
        if(read(fdr,&pck,sizeof(struct communication_list_handler_pck)) <= 0){
            perror("Errore nella lettura\n");
            continue;
        }

        //printf("Pacchetto arrivato. TIPO : %d\n",pck.type);

        //Controllo l'operazione richiesta
        switch(pck.type){

            case 0: //Inserisco una nuova comunicazione
                ptr = (unsigned char*)pck.value;
                client1 = *((struct client**) (ptr)); ptr = ptr + sizeof(struct client*);
                client2 = *((struct client**) (ptr)); ptr = ptr + sizeof(struct client*);
                tr = *((pthread_t*)ptr); ptr = ptr + sizeof(pthread_t);
                fdw = *((int*)ptr);
                communication_add(&list_communication,client1,client2,tr,fdw);
                break;

            case 1: //Elimino un nodo dato il communication id
                id = *((int*)pck.value);
                communication_remove_id(&list_communication,id);
                break;

            case 2: //Elimino un nodo dato il thread id
                tr = *((pthread_t *)pck.value);
                communication_remove_tr(&list_communication,tr);
                break;

            case 3: //Restituisce l'indirizzo della struct communication dato il communication id
                id = *((int*)pck.value);
                comm = addr_get_communication_id(&list_communication,id);
                //Invio il risultato
                write(pck.fdw,&comm,sizeof(struct communication*));
                close(pck.fdw);
                break;

            case 4: //Restituisce l'indirizzo della struct communication dato il thread id
                tr = *((pthread_t*)pck.value);
                comm = addr_get_communication_tr(&list_communication,tr);
                
                //Invio il risultato
                write(pck.fdw,&comm,sizeof(struct communication*));
                close(pck.fdw);
                break;

            case 5: //resituisce il thread_id di una comunicazione deto il communication id
                id = *((int*)pck.value);
                tr = communication_get_tr(&list_communication, id);
                //Invio i risultati
                write(pck.fdw,&tr,sizeof(pthread_t));
                close(pck.fdw);
                break;

            case 6:
                tr = *((pthread_t*)pck.value);
                id = communication_get_id(&list_communication, tr);
                //Invio i risultati
                write(pck.fdw,&id,sizeof(int));
                close(pck.fdw);
                break;

            case 7:
                //work in progress
                break;
            case 8:
                communication_print_list(&list_communication);

            default:

                break;
        }
    }
}

///////////////////////////////
//Other function
