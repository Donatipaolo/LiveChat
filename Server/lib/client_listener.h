#ifndef CLIENT_LISTENER_H
#define CLIENT_LISTENER_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <sys/select.h>

#define PORT 5700

extern int client_handler_id;
extern int thread_handler_id;
extern fd_set fd;
extern int fd_max;

//Questa componente si occupa di rimanere in ascolto e registrare i nuovi client

//struct for parameters
struct parameters{
    int file_descriptor;
    int boolean;
    int pipefdr;
};


int initSocket(int domain, int type, int protocol, int IP, int port,struct sockaddr_in** addr_r);
void* client_listener(void* arg);
void* init_client(void *arg);
ssize_t read_exact(int fd, void *buf, size_t count);
void read_form_fd(int sfd);

#endif