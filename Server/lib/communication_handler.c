#include "./communication_handler.h"


//////////////////////////////////////////////////////////////////////////////////////////////////////////////
//COMMUNICATION HANDLER FUNCTION
void* communication_handler(void* args){
    //leggo dagli argomenti l'fd della pipe;
    int fdr = *((int*)args);
    free(args);
    pthread_t tr;
    int bytes;

    //Leggo il mio thread identifiear
    read(fdr,&tr,sizeof(pthread_t));
 
    fflush(stdout);

    //Inizializzo le variabili della comuncazione
    struct communication* comm;
    while((comm = get_communication_addr_tr(tr)) == NULL){sleep(1);}
    struct client* client1 = comm->client1;
    struct client* client2 = comm->client2;

    
    //Inizializziamo i FD SET per la select
    //Dichiaramo due fd_set
    fd_set fd, temp_fd;

    //Inizializzo i file set
    FD_ZERO(&fd);
    FD_SET(fdr,&fd);
    FD_SET(client1->fd,&fd);
    FD_SET(client2->fd,&fd);

    //Inizializzo l'array di fd_set
    int fd_set_array[FD_SET_SIZE] = {fdr,client1->fd,client2->fd};
    int max_fd = get_max_fd(fd_set_array,FD_SET_SIZE);
    //Inizializzazione delle variabili per il pacchetto
    struct communication_handler_pck pck;
    int fd_to_wr;

    while(1){//Inizio del ciclo

        //Inizializzo fd_set temporaneio
        temp_fd = fd;

        fflush(stdout);
        //Aspetto con select
        if(select(max_fd+1,&temp_fd,NULL,NULL,NULL)  < 0){
            //closing route
            perror("select communication Handler");
            pthread_exit(NULL);
        }


        //Controllo quali file descriptor sono attivi
        for(int i = 0; i < FD_SET_SIZE; i++){
            //Controllo se è attivo
            if(FD_ISSET(fd_set_array[i],&temp_fd)){
                if(i == 0){
                    //Comunicazione con il server Request Handler
                    if ((bytes = read(fd_set_array[i],&pck,sizeof(struct communication_handler_pck))) < 0) {
                        perror("read failed");
                        communication_closing_routine(client1, client2, tr);
                        pthread_exit(NULL);
                    }

                    switch (pck.type)
                    {
                    case 1:
                        communication_closing_routine(client1,client2,tr);
                        pthread_exit(NULL);
                        break;
                    default:
                        break;
                    }

                } else {
                    //Messaggio da parte di un client

                    //Leggo il pacchetto
                    if ((bytes = read(fd_set_array[i],&pck,sizeof(struct communication_handler_pck))) < 0) {
                        perror("read failed");
                        communication_closing_routine(client1, client2, tr);
                        pthread_exit(NULL);
                    }


                    if(bytes == 0){
                        communication_closing_routine(client1,client2,tr);
                        pthread_exit(NULL);
                    }

                    //Controllo il tipo del pacchetto
                    switch(pck.type){
                        case 0://Messaggio normale
                            //Invio il messaggio al secondo client
                            if(i == 1){
                                fd_to_wr = 2;
                            }
                            else {
                                fd_to_wr = 1;
                            }

                            if (write(fd_set_array[fd_to_wr], &pck, sizeof(pck)) <= 0) {
                                perror("write failed");
                                communication_closing_routine(client1, client2, tr);
                                pthread_exit(NULL);
                            }
                            break;
                        case 1://Richista di interruzione
                            communication_closing_routine(client1,client2,tr);
                            pthread_exit(NULL);
                            break;
                        default:
                            break;
                    }
                }
            }
        }
    }
}

void communication_closing_routine(struct client* client1,struct client* client2,pthread_t tr){
    //Rimuovo la entry nella comunication list
    remove_communication_tr(tr);

    //Invio il messaggio di terminazione ai due client
    //Creo il pacchetto
    struct communication_handler_pck pck;
    pck.type = 1;

    //Invio il messaggio ai due client
    write(client1->fd,&pck,sizeof(struct communication_handler_pck));
    write(client2->fd,&pck,sizeof(struct communication_handler_pck));

    //aspetto qualche microsecondo
    usleep(1000);

    //Aggiorno i loro status
    client1->status = 0;
    client2->status = 0;

    //Aggiorno la file_descriptor_list del server_request_handler
    //Work in progress
    	
    //Reimposto gli fd
    FD_SET(client1->fd,&fd);
    FD_SET(client2->fd,&fd);

    //Comunico al thread handler di togliere questo thread
    remove_thread(tr);
}   

int get_max_fd(int fds[],size_t size){
//Ricerca del massimo

    if(size == 0) return -1;

    if(size == 1) return fds[0];

    //Salvo il primo valore e poi controllo
    int max = fds[0];

    for(int i = 1; i < size; i++){
        if(max < fds[i])
            max = fds[i];
    }
    return max;
}

void wait_key_press(){
    printf("Premi Invio per continuare...");
    getchar();
}
