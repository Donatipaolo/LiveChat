#ifndef COMMUNICATION_HANDLER_H
#define COMMUNICATION_HANDLER_H

////////////////////////////////////////////////////////////////////////////////
//Include and definition
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/select.h>
#include "client_handler.h"
#include "communication_list_handler.h"
#include "mthread_handler.h"

//DIMENSIONE MASSIMA DEL MESSAGGIO
#define MAX_MSG_LENGHT 1024

//DIMENSIONE MASSIMA DEL FD_SET
#define FD_SET_SIZE 3

extern fd_set fd;

///////////////////////////////////////////////////////////////////////////////
//STRUCT 

struct communication_handler_pck{
    int type;
    int len;
    uint8_t bytes[MAX_MSG_LENGHT];
};

//////////////////////////////////////////////////////////////////////////////
//Function
void* communication_handler(void* args);
void communication_closing_routine(struct client* client1,struct client* client2,pthread_t tr);
int get_max_fd(int fds[],size_t size);
void wait_key_press();

#endif
