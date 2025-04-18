#include "mthread_handler.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h> 

struct mthread_list *list_mthread = NULL;

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//Funzione che aggiunge un thread alla lista di thread
void create_thread(pthread_t id, int fdw, int type){
    //Inizializzo il pacchetto
    struct mthread_handler_pck pck;
    pck.type = 0;
    pck.len = sizeof(struct mthread);

    //Inizializzo il thread
    struct mthread tr;
    tr.fdw = fdw;
    tr.id = id;
    tr.type = type;

    //Inserisco il thread nel pacchetto
    memcpy((void*) pck.value, (void*) &tr, sizeof(struct mthread));

    //Mando il pacchetto
    write(thread_handler_id,&pck,sizeof(struct mthread_handler_pck));
}

//Funzione che rimuove un thread dalla listta dato il thread id
void remove_thread(pthread_t id){
    //Inizializzo il pacchetto
    struct mthread_handler_pck pck;

    //Inserisco i valori del pacchetto
    pck.type = 1;
    pck.len = sizeof(pthread_t);
    memcpy(pck.value,(void*)&id,sizeof(pthread_t));

    //Invio la richiesta
    write(thread_handler_id,&pck,sizeof(struct mthread_handler_pck));
}

//Funzione che restituisce l'indirizzo id un thread dato l'id
struct mthread* get_thread(pthread_t id){
    //Inizializzo il pacchetto
    struct mthread_handler_pck pck;

    //Inizializzo il puntatore a thread
    struct mthread* tr;

    //Inizializzo il canale di comunicazione
    int pipefd[2];
    pipe(pipefd);

    //Inserisco i valori del pacchetto
    pck.type = 2;
    pck.len = sizeof(pthread_t);
    memcpy(pck.value,(void*)&id,sizeof(pthread_t));
    pck.fdw = pipefd[1];

    //Invio la richesta
    write(thread_handler_id,&pck,sizeof(struct mthread_handler_pck));

    //leggo la risposta
    read(pipefd[0],&tr,sizeof(struct mthread*));

    //chiudo la comuncazione
    close(pipefd[0]);

    return tr;
}

//Funzione che stampa la lista dei thread nel server
void server_print_thread(){
    //Inizializzo il pacchetto
    struct mthread_handler_pck pck;
    pck.type = 6;

    //printf("Sto scrivendo un pacchetto di tipo : %d",pck.type);
    //Invio la richiesta
    write(thread_handler_id,&pck, sizeof(struct mthread_handler_pck));
}


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void read_mthread(struct mthread_handler_pck pck, pthread_t *id,int *fdw, int *type){
    struct mthread tr;
    memcpy((void*)&tr,(void*)pck.value,MAX_THREAD_PCK_LENGHT);

    *fdw = tr.fdw;
    *type = tr.type;
    *id = tr.id;
}

struct mthread read_struct_mthread(struct mthread_handler_pck pck){
    struct mthread tr;
    memcpy((void*)&tr,(void*)pck.value,MAX_THREAD_PCK_LENGHT);
    return tr;
}

void mthread_enqueue(struct mthread_list **head, struct mthread tr) {
    struct mthread_list *new_node = malloc(sizeof(struct mthread_list));
    if (!new_node) {
        perror("malloc");
        return;
    }
    new_node->tr = tr;
    new_node->next = NULL;

    if (*head == NULL) {
        *head = new_node;
    } else {
        struct mthread_list *current = *head;
        while (current->next != NULL) {
            current = current->next;
        }
        current->next = new_node;
    }
}

void mthread_remove(struct mthread_list **head, pthread_t id) {
    struct mthread_list *current = *head;
    struct mthread_list *prev = NULL;

    while (current != NULL) {
        if (pthread_equal(current->tr.id, id)) {
            if (prev == NULL) {
                *head = current->next;
            } else {
                prev->next = current->next;
            }
            close(current->tr.fdw); // chiude il file descriptor
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

struct mthread* mthread_get(struct mthread_list *head, pthread_t id) {
    struct mthread_list *current = head;
    while (current != NULL) {
        if (pthread_equal(current->tr.id, id)) {
            return &(current->tr);
        }
        current = current->next;
    }
    return NULL;
}

//void closing_routin(); //SOLO LATO SERVER DA FARE

void print_mthread(struct mthread_list *node) {
    if (node != NULL) {
        printf("Thread ID: %lu | FD: %d | Type: %d\n", node->tr.id, node->tr.fdw, node->tr.type);
    }
}

// Stampa la lista di tutti i thread
void print_mthread_list(struct mthread_list *head) {
    struct mthread_list *current = head;
    printf("Lista dei thread attivi:\n");
    while (current != NULL) {
        print_mthread(current);
        current = current->next;
    }
}

//THREAD HANDLER FUNCTION
void* mthread_handler(void* arg){
    int fdr = *((int*)arg);
    free(arg);
    //printf("Leggo dal fd : %d\n",fdr);
    struct mthread *tr;
    struct mthread_handler_pck pck;
    int fdw;
    size_t dim;
    //printf("SIZE OF MTHREAD HANDLER PCK : %ld\nSIZE OF MTHREAD : %ld\nSIZE OF PTHREAD_T : %ld\n",sizeof(struct mthread_handler_pck),sizeof(struct mthread),sizeof(pthread_t));

    //Inizio del ciclo
    while(1){

        if((dim = read(fdr, &pck, sizeof(struct mthread_handler_pck))) < 0){
            perror("Mthread-handler Read");
            pthread_exit(NULL);
        }
        //printf("Numero di byte letti : %ld\nTipo del pacchetto letto %d\n",dim,pck.type);
        switch(pck.type){

            case 0://creazione di un nuovo thread
                mthread_enqueue(&list_mthread, read_struct_mthread(pck));
                // printf("Accodamento corretto\n");
                break;
            case 1://Rimozione di un thread dato l'id
                mthread_remove(&list_mthread,*((pthread_t*)pck.value));
                break;
            case 2:
                tr = mthread_get(list_mthread,*((pthread_t*)pck.value));//Parametro : id
                fdw = pck.fdw;
                write(fdw,(void*)&tr,sizeof(struct mthread*));
                close(fdw);
                break;
            case 3:
                close(fdr);
                pthread_exit(NULL);
                break; //work in progress
            case 6:
                print_mthread_list(list_mthread);
                break;
            default:
                break;
        }
    }
}
