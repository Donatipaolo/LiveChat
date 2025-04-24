#ifndef CLIENT_H
#define CLIENT_H

//Include
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdint.h>


//Definitions
#define USERNAME_LENGHT 100
#define MAX_MSG_LENGHT 1024
#define MAX_LENGHT_SERVER_REQUEST_PCK 1024

extern struct client_info **clients;
extern int clients_dim;
extern int clients_current;

//Struct per la comunicazione

//Pacchetto per inviare dati al server
struct server_request_handler_pck{
    int type;
    uint8_t bytes[MAX_LENGHT_SERVER_REQUEST_PCK];
    size_t len;
};

struct int_and_username{
    int integer;
    char buffer[USERNAME_LENGHT];
};

struct communication_handler_pck{
    int type;
    int len;
    uint8_t bytes[MAX_MSG_LENGHT];
};

struct client_info{
    char buffer[USERNAME_LENGHT];
    int status;
};

//Funzioni del client

////////////////////////////////////////////////////////////////////////////////////////////////////
//Funzioni 

void change_username(int sockfd);
void communication(int sockfd,char buffer[USERNAME_LENGHT]);
void update_client(int sockfd);//Funzione che aggiorna i client
void request_handler(int sockfd);
int server_clear(int sockfd,int seconds,int microseconds);
int choose_client(int sockfd);
void handle_server_request(int sockfd);
void handle_connection_request(int sockfd,int index);
void handle_connection_request_ricezione(int sockfd, struct server_request_handler_pck* pck);

////////////////////////////////////////////////////////////////////////////////////////////////////
//Funzioni per inviare messaggio al server

void send_connection_request(int sockfd,char buffer[USERNAME_LENGHT]);
void send_connection_accept(int sockfd,struct server_request_handler_pck *pck);
void send_connection_refuse(int sockfd,struct server_request_handler_pck *pck);
void send_client_update(int sockfd);
void send_client_exit(int sockfd);
void send_communication_msg(int sockfd, char buffer[MAX_MSG_LENGHT],struct communication_handler_pck* pck);
void send_communication_exit(int sockfd,struct communication_handler_pck* pck);
void wait_for_communication_exit(int sockfd,struct communication_handler_pck* pck);
void wait_for_start_communication(int sockfd);

////////////////////////////////////////////////////////////////////////////////////////////////////
//Funzioni per la parte grafica
void print_client(struct client_info **info, int n_client);
void print_communication_wait(char buffer[USERNAME_LENGHT]);
void print_communication_accept(char buffer[USERNAME_LENGHT]);
void print_communication_refuse(struct server_request_handler_pck *pck,char buffer[USERNAME_LENGHT]);
void print_communication_request(char buffer[USERNAME_LENGHT]);
void print_init_communication(char buffer[USERNAME_LENGHT]);
void print_my_msg(char buffer[MAX_MSG_LENGHT]);
void print_other_msg(char usr[USERNAME_LENGHT],char buffer[MAX_MSG_LENGHT]);

///////////////////////////////////////////////////////////////////////////////////////////////////
//Altre funzioni
void change_clients_dim(int dim);
void wait_key_press();
void clear_last_lines(int n);
void clear_input_buffer();
void strip_newline(char *str);

//Per prendere in input in maniera sicura le stringhe
int safe_fgets(char *buffer,ssize_t dim);

#endif