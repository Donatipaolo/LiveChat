#include <stdio.h>
#include <sys/resource.h>
#include "lib/client_handler.h"
#include "lib/client_listener.h"
#include "lib/communication_handler.h"
#include "lib/communication_list_handler.h"
#include "lib/mthread_handler.h"
#include "lib/server_request_handler.h"

int thread_handler_id;
int client_handler_id;
int communication_list_handler_id;
int server_request_handler_id;

fd_set fd,temp_fd;
int fd_max;

//InitFunction
void init_server();
void init_client_handler();
void init_client_listener();
void init_communication_list_handler();
void init_mthread_handler();
void init_server_request_handler();
void close_all_fds_except(int fd_to_keep);

int main(){
    init_server();

    printf("Server is working!!\n");

    char buffer[10];
    scanf("%s",buffer);
    // close_all_fds_except(-1);

    return 0;
}



void init_server(){

    //Inizializzazione delle varie componenti

    init_mthread_handler();

    init_client_handler();

    init_communication_list_handler();

    init_server_request_handler();

    init_client_listener();
}

void init_client_handler(){
    pthread_t id;

    //inizializzo la pipe
    int *pipefd = (int*)malloc(sizeof(int)*2);
    pipe(pipefd);

    //Creo il thread
    if(pthread_create(&id,NULL,client_handler,&pipefd[0]) < 0){
        perror("Client Init");
        exit(EXIT_FAILURE);
    }

    client_handler_id = pipefd[1];

    //Inserisco il thread
    create_thread(id,client_handler_id,0);
    //free(&pipefd[1]);

}

void init_client_listener(){
    pthread_t id;

    //Creo il thread
    if(pthread_create(&id,NULL,client_listener,NULL) < 0){
        perror("CLient listener Init");
        exit(EXIT_FAILURE);
    }

    //Inserisco il suo thread_Id
    create_thread(id, -1, 0);
}

void init_communication_list_handler(){
    pthread_t id;

    //inizializzo la pipe
    int *pipefd = (int*)malloc(sizeof(int)*2);
    pipe(pipefd);

    //Creo il thread
    if(pthread_create(&id,NULL,communication_list_handler,&pipefd[0]) < 0){
        perror("Communication list handler");
        exit(EXIT_FAILURE);
    }

    communication_list_handler_id = pipefd[1];

    //Inserisco il suo thread_Id
    create_thread(id, communication_list_handler_id, 0);
    //free(&pipefd[1]);
}

void init_mthread_handler(){
    pthread_t id;

    //inizializzo la pipe
    int *pipefd = (int*)malloc(sizeof(int)*2);
    pipe(pipefd);

    //Creo il thread
    if(pthread_create(&id,NULL,mthread_handler,&pipefd[0]) < 0){
        perror("Mthread Init");
        exit(EXIT_FAILURE);
    }

    thread_handler_id = pipefd[1];

    //Inserisco il suo thread_Id
    create_thread(id, thread_handler_id, 0);
    //free(&pipefd[1]);

}

void init_server_request_handler(){
    pthread_t id;

    //inizializzo la pipe
    int *pipefd = (int*)malloc(sizeof(int)*2);
    pipe(pipefd);

    //Creo il thread
    if(pthread_create(&id,NULL,server_request_handler,&pipefd[0]) < 0){
        perror("Server request Handler init");
        exit(EXIT_FAILURE);
    }

    server_request_handler_id = pipefd[1];

    //Inserisco il suo thread_Id
    create_thread(id, server_request_handler_id, 0);
}


void close_all_fds_except(int fd_to_keep) {
    struct rlimit rl;
    if (getrlimit(RLIMIT_NOFILE, &rl) == -1) {
        perror("getrlimit");
        return;
    }

    for (int fd = 3; fd < rl.rlim_max; fd++) {
        if (fd != fd_to_keep) {
            close(fd);
        }
    }
}
