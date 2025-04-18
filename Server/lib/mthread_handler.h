#ifndef MTHREAD_HANDLER_H
#define MTHREAD_HANDLER_H

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>
/*
Il thread handler è quella componente che si occupa di aggiungere e gestire la coda dei thread
La coda dei thread permette a ciascun thread di individuare gli altri thread, permettendo la comunicazione attraverso una pipe
E permette la terminazione dei thread
*/

extern int thread_handler_id;
extern int client_handler_id;

struct mthread{
    pthread_t id; //id del thread
    int fdw; //file descriptor della pipe per la comunicazione
    int type; //tipo di thread
};

#define MAX_THREAD_PCK_LENGHT sizeof(struct mthread) 



//FUNZIONI E STRUCT PER LA COMUNICAZIONE

struct mthread_handler_pck{
    int type;
    unsigned char value[MAX_THREAD_PCK_LENGHT];
    int len;
    int fdw;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
//API
void create_thread(pthread_t id, int fdw, int type);
void remove_thread(pthread_t id);
struct mthread* get_thread(pthread_t id);
void server_print_thread();


void read_mthread(struct mthread_handler_pck pck, pthread_t *id,int *fdw, int *type);
struct mthread read_struct_mthread(struct mthread_handler_pck pck);

///////////////////////////////////////////////////////////////////////////////////////////////////
//FUNZIONI PER LA GESTIONE DELLE LISTE
//I tipi di richieste che possono essere fatto al thread handler sono:
// 0) aggiungi thread (in coda)
// 1) elimina thread (con valore)
// 2) restituisci thread (dato ID)
// 3) Closing routin (Insieme di procedure che permettono la chiusura di tutti i thread)
// 6) Stampa la lista dei thread nel server


struct mthread_list{
    struct mthread tr;
    struct mthread_list *next;
};

///////////////////////////////////////////////////////////////////////////////////////////////////

void mthread_enqueue(struct mthread_list **head, struct mthread tr);
void mthread_remove(struct mthread_list **head, pthread_t id);
struct mthread* mthread_get(struct mthread_list *head,pthread_t id);
//void closing_routin();
void print_mthread(struct mthread_list *head);
void print_mthread_list(struct mthread_list *head);

//THREAD HANDLER FUNCTION
void* mthread_handler(void* arg);

#endif