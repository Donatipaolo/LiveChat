#ifndef COMMUNICATION_LIST_HANDLER_H
#define COMMUNICATION_LIST_HANDLER_H

#include "client_handler.h"
#include <pthread.h>
#define MAX_COMMUNICATION_PCK_LENGHT sizeof(struct communication)
//Componente che per la gestione della communication list

extern int communication_list_handler_id;
///////////////////////////////////////////////////////////////////////////////////////////////////////////////
//DEFINIZIONE DELLE STRUCT UTILIZZATE

//Comunicazione 
struct communication{
    struct client* client1; //primo client gestito dalla comunicazione
    struct client* client2; //secondo client gestito dalla comunicazione
    int communication_id; //id della comunicazione
    pthread_t id;  //id del thread che gestisce la comunicazione
    int fdw; //Pipe per la comunicazione tra thread
};

//Pacchetto da inviare al communication handler
struct communication_list_handler_pck{
    int type;//tipo di messaggio
    unsigned char value[MAX_COMMUNICATION_PCK_LENGHT];//Valori passati
    int len;//grandezza utilizzata
    int fdw; //Se necessario
};

//TIPI DI PACCHETTI:
// 0 - Inserisci una nuova comunicazione
// 1 - Rimuovi una comunicazione (dato comunication id)
// 2 - Rimuovi una comunicazione (dato il thread id del thread che la gestisce)
// 3 - Riceve l'indirizzo della struct communication dato id
// 4 - Riceve l'indirizzo della struct communication dato il thread id
// 5 - Restituisce il thread id di una comunicazione data la communication id
// 6 - Stampa a schermo la lista delle comunicazioni aperte (lato server)
// 7 - closing route


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//API
void create_communication(struct client* client1,struct client* client2,pthread_t id, int fdw);
void remove_communication_id(int id);
void remove_communication_tr(pthread_t tr);
struct communication* get_communication_addr_id(int id);
struct communication* get_communication_addr_tr(pthread_t tr);
pthread_t get_communication_tr(int id);
int get_communication_id(pthread_t tr);
void print_communication_list();
//void closing_route()


/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//LIST FUNCTION AND STRUCT

//struct
struct communication_list{
    struct communication* com;
    struct communication_list* next;
};

//function

void communication_add(struct communication_list** head,struct client* client1,struct client* client2,pthread_t id, int fdw);
void communication_remove_id(struct communication_list** head,int id);
void communication_remove_tr(struct communication_list** head,pthread_t tr);
struct communication* addr_get_communication_id(struct communication_list** head,int id);
struct communication* addr_get_communication_tr(struct communication_list** head,pthread_t tr);
pthread_t communication_get_tr(struct communication_list** head,int id);
int communication_get_id(struct communication_list** head,pthread_t tr);
void communication_print_list(struct communication_list** head);


///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//COMMUNICATION LIST HANDLER FUNCTION
void* communication_list_handler(void* arg);



#endif