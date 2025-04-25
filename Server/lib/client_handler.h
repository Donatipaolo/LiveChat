#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H
#define USERNAME_LENGHT 100
#include <stdlib.h>
#include "client_listener.h"
#include "mthread_handler.h"

extern int client_handler_id;

//Sturct
struct client{
    int fd; //file descriptro of the socket
    char username[USERNAME_LENGHT]; //username
    int status; //free, busy, ...
    //struct client* com;
};

struct client_info{
    char buffer[USERNAME_LENGHT];
    int status;
};

#define MAX_CLIENT_PCK_LENGHT sizeof(struct client)

struct client_handler_pck{
    int type; //define the type of the package
    unsigned char value[MAX_CLIENT_PCK_LENGHT]; //it depends on the type
    int len; //how many bytes we actually used
    int fdw; //Se necessario, alrimenti -1
};

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//API
void create_client(int fd, char buffer[]);
void remove_client(int fd);
struct client* get_client(int fd);
int free_client(int fd);
int exists_client(int fd);
void server_print_client();
int is_taken_client(char buffer[USERNAME_LENGHT]);
void change_username_client(int fd,char buffer[USERNAME_LENGHT]);
struct client* get_client_by_username(char buffer[USERNAME_LENGHT]);
void clients_update_pck(int sockfd);
void change_username_server(int clientfd);


void read_client(struct client_handler_pck pck,int* fd, char *buffer, int* status);
struct client read_struct_client(struct client_handler_pck pck);

///////////////////////////////////////////////////////////////////////////////////////////////////////////
//LIST
struct client_list{
    struct client * cli;
    struct client_list* next;
};



//LIST FUNCTION
void client_push(struct client_list** head, struct client cli);
void client_remove(struct client_list** head, int fd); //rimozione secondo il campo fd della struct client
struct client* client_get(struct client_list* head, int fd);
int client_exists(struct client_list* head, int fd);
int client_free(struct client_list* head, int fd);
void print_client(const struct client* c);
void print_client_list(const struct client_list* head);
int username_taken(const struct client_list* head,const char buffer[USERNAME_LENGHT]);
void change_username(const struct client_list* head,const char buffer[USERNAME_LENGHT], int fd);
struct client* client_get_by_username(struct client_list* head,const char buffer[USERNAME_LENGHT]);
int size(struct client_list* head);
//CLIENT HANDLER FUNCTION
void* client_handler(void* arg);

/////////////////////////////////////////////////////////////////////////////////////////////////////////
//Function for clients update

void send_all_username(int sockfd);
int size(struct client_list* head);

//MESSAGE STRUCT
struct username_pck{
    char buffer[USERNAME_LENGHT];
    int fd;
};

#endif