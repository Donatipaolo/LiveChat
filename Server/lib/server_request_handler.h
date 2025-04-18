#ifndef SERVER_REQUEST_HANDLER_H
#define SERVER_REQUEST_HANDLER_H
//GESTORE DELLE SERVER REQUEST


///////////////////////////////////////////////////////////////////////////////////
//LIBRERIE E DEFINIZIONI
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdint.h>
#include "client_handler.h"
#include "communication_handler.h"
#include <sys/select.h>
#include "client_listener.h"

#define MAX_LENGHT_SERVER_REQUEST_PCK 1024 //temporaneo

extern fd_set fd,temp_fd;
extern int fd_max;


///////////////////////////////////////////////////////////////////////////////////
//STRUCT 

struct server_request_handler_pck{
    int type;
    uint8_t bytes[MAX_LENGHT_SERVER_REQUEST_PCK];
    size_t len;
};

//Per la gestione delle richieste di connessione
struct request{
    struct client* requester;
    struct client* receiver;
    int request_id;
};

//Struct per la gestione delle lista
struct request_list{
    struct request* req;
    struct request_list* next;
};

/////////////////////////////////////////////////////////////////////////////////////////
//API (Funzioni per inviare pacchetti per le richieste ai client da parte del server)
void send_connection_request(int fdw,char* buffer,int request_id);
void send_connection_accept(int fdw,char* buffer,int request_id);
void send_connection_refuse(int fdw,char* buffer,int request_id);
void send_start_communication(int fdw);

//////////////////////////////////////////////////////////////////////////////////////////
//FUNZIONI PER LA GESTIONE DELLE RICHIESTE
void* server_request_handler(void* args);

//GESTIONE DELLA COMUNICAZIONE INTERNA ED ESTERNA
void handler_pipe(int fdr,struct server_request_handler_pck* pck);
void handle_sockets(int fdr,struct server_request_handler_pck* pck);

//GESTIONE DELLE RICHIESTE DI CONNESSIONE
void create_connection_request(struct server_request_handler_pck* pck,struct client* req,int sfd);
void accept_connection_request(struct server_request_handler_pck* pck);
void refuse_connection_request(struct server_request_handler_pck* pck);

//CREAZIONE DELLA COMUNICAZIONE
void init_communication(struct client* requester, struct client* receiver);

//RIMOZIONE DI UN CLIENT
void remove_client_server(struct server_request_handler_pck* pck,struct client* req);

//MODIFICA DEL USERNAME
void server_change_username(struct server_request_handler_pck* pck, int fdr);

//ROUTINE DI CHIUSURA
void server_handler_closing_routine();

///////////////////////////////////////////////////////////////////////////////////////////
//FUNZIONI PER LA GESTIONE DELLA LISTA
//ADD REQUEST
//REMOVE REQUEST
//GET REQUEST

void add_request(struct request_list** head,struct client* requester, struct client* receiver, int id);
void remove_request(struct request_list** head,int id);
struct request* get_request(struct request_list* head,int id);
void print_request(struct request* req);
void print_request_list(struct request_list* head);

///////////////////////////////////////////////////////////////////////////////////////////
//OTHER
int get_new_id();//?


///////////////////////////////////////////////////////////////////////////////////////////
//OTHER STRUCT 
struct int_and_username{
    int integer;
    char buffer[USERNAME_LENGHT];
};

#endif