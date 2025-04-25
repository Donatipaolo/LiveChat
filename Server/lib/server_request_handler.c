#include "server_request_handler.h"
#include <string.h>

struct request_list* list_request = NULL;

int current_new_request_id = 0;

//////////////////////////////////////////////////////////////////////////////////////////
//FUNZIONI PER LA GESTIONE DELLE RICHIESTE
void* server_request_handler(void* args){
    //Inizializzo le variabili
    int fdr = *((int*)args); //Read end pipe
    free(args);
    
    struct server_request_handler_pck pck;

    //Timeout
    struct timeval timeout;
    timeout.tv_sec = 1;
    timeout.tv_usec = 0;

    //Inizializzazione dei SETFD
    FD_ZERO(&fd);

    //Inserisco LA PIPE NEL SETFD
    FD_SET(fdr,&fd);

    //Variabili per il fd_set
    fd_max = fdr;
    //int fd_set_value[]; Da rivedere

    //Inizio dei controlli
    while(1){
        //Questo perchè select distrugge il setfd passato
        temp_fd = fd;

        if(select(fd_max+1,&temp_fd,NULL,NULL,&timeout) < 0){
            perror("Select server request handler");
            exit(EXIT_FAILURE);
        }

        //printf("Select done\n");

        //Controllo gli fd
        for(int i = 3; i <= fd_max; i++){
            //Controllo se posso leggere qualcosa
            if(FD_ISSET(i,&temp_fd)){
                //controllo se è da parte della pipe o da un client
                if(i == fdr){
                    //Handle for pipe
                    handler_pipe(i,&pck);

                }else{
                    //Handle for socket
                    //Leggo il pacchetto dalla pipe
                    handle_sockets(i,&pck);
                }
            }
        }
    }
}

void handler_pipe(int fdr,struct server_request_handler_pck* pck){
    //Leggo il messaggio
    if(read(fdr,pck,sizeof(struct server_request_handler_pck)) < 0){
        perror("server handler read");
        return;
    }

    switch(pck->type){
        case 7:
            server_handler_closing_routine();
            break;
        default:
        break;
    }
}

void handle_sockets(int fdr,struct server_request_handler_pck* pck){
    //Leggo il pacchetto
    struct client* cli;
    int bytes;
    cli = get_client(fdr);

    if((bytes = read(fdr,pck,sizeof(struct server_request_handler_pck))) < 0){
        perror("server handler read");
        return;
    }

    if(bytes == 0){
        remove_client_server(pck,cli);
        return;
    }


    switch(pck->type){
        case 0:
            create_connection_request(pck,cli,fdr);
            break;
        case 1:
            accept_connection_request(pck);
            break;
        case 2:
            refuse_connection_request(pck);
            break;
        case 3:
            server_change_username(pck,fdr);
            break;
        case 4:
            remove_client_server(pck,cli);
            break;
        case 5:
            //Code for update clients
            clients_update_pck(fdr);
            break;
        case 6: 
            //Change username pck
            change_username_server(fdr);
            break;
        default:
        break;
    }
}

//GESTIONE DELLE RICHIESTE DI CONNESSIONE
void create_connection_request(struct server_request_handler_pck* pck,struct client* req, int sfd){

    //Controllo se esiste il client ricevente
    struct client* cli = get_client_by_username((char*)pck->bytes); //Primi 100 bytes
    char *buffer = NULL;
    int status = -1;
    //printf("Nome letto : %s\n",(char*)pck->bytes);
    
    if(cli != NULL){
        //Se esiste avvio la procedura
        buffer = cli->username;
        status = -2;

        if(cli->status == 0){
            req->status = 1;
            cli->status = 1;
            
            //Genero un altro id
            int id = get_new_id();//Funzione da fare (molto semplice)
            
            //Creo una nuova entry nella request list
            add_request(&list_request,req,cli,id);

            //Invio un pacchetto di connection request al secondo client
            send_connection_request(cli->fd,req->username,id);

            //Tolgo il file descriptor dal setfd (del requester)
            FD_CLR(sfd,&fd);

            return;
        }
    } 

    //GLI STATI POSSIBILI SONO : 
    //-1 SE IL CLIENT NON È STATO TROVATO
    //-2 SE IL CLIENT È OCCUPATO

    //Nei restanti casi invio un pacchetto di tipo 2 connection refused
    send_connection_refuse(req->fd,buffer,status);
}

void accept_connection_request(struct server_request_handler_pck* pck){
    //Controllo di quale request stiamo parlando
    struct int_and_username* bytes = (struct int_and_username*) pck->bytes;
    struct request* req = get_request(list_request,bytes->integer);

    //Modifico gli status dei due client
    req->receiver->status = 2;
    req->requester->status = 2;

    //Comunico al client che la comunicazione è stata accettata
    send_connection_accept(req->requester->fd,req->receiver->username,bytes->integer);
    
    struct client* requester = req->requester;
    struct client* receiver = req->receiver;

    //Elimino la entry dalla request list
    remove_request(&list_request,req->request_id);
    

    //Inizializzo la comunicazione
    init_communication(requester,receiver);

}

void refuse_connection_request(struct server_request_handler_pck* pck){
    //Controllo di quale request stiamo parlando
    struct int_and_username* bytes = (struct int_and_username*) pck->bytes;
    struct request* req = get_request(list_request,bytes->integer);

    //Imposto gli status a free
    req->receiver->status = 0;
    req->requester->status = 0;

    //Invio al primo client un refuse connection
    send_connection_refuse(req->requester->fd,req->receiver->username,req->request_id);

    //Reimposto il suo fd del set fd
    FD_SET(req->requester->fd,&fd);

    //Elimino la richiesta
    remove_request(&list_request,req->request_id);
}

//CREAZIONE DELLA COMUNICAZIONE
void init_communication(struct client* requester, struct client* receiver){
    //Modifico i set fd
    FD_CLR(receiver->fd,&fd);
    FD_CLR(requester->fd,&fd);

    //Genero una pipe per il communication handler
    int *pipefd = malloc(sizeof(int)*2);
    pipe(pipefd);
    int fdw = pipefd[1];

    //thread id
    pthread_t tr;

    //Genero un altro thread per la comunicazione
    if(pthread_create(&tr,NULL,communication_handler,&pipefd[0]) < 0){
        perror("Errore nella creazione del thread nella funzione init_communication");
        exit(1);
    }
    
    //Mando una richiesta alla communication list
    create_communication(requester,receiver,tr,pipefd[1]);
    usleep(100);

    //Comunico al thread il proprio thread id
    write(fdw,&tr,sizeof(pthread_t));

    //Comunico ai due client che la comunicazione è pronta
    send_start_communication(requester->fd);
    send_start_communication(receiver->fd);
}

//RIMOZIONE DI UN CLIENT
void remove_client_server(struct server_request_handler_pck* pck,struct client* req){
    //Disattivo il file descriptor
    FD_CLR(req->fd,&fd);

    //Lo elimino dalla lista di client
    remove_client(req->fd);

    //chiudo la comunicazione 
    close(req->fd);
}

//MODIFICA DEL USERNAME
void server_change_username(struct server_request_handler_pck* pck, int fdr){
    //Funzione che permette di cambiare username

    //Creo un thread in modo da non bloccare il programma
    //Riutilizzo una funzione in client_listener
    //Inizializzazione della variabili
    pthread_t id;
    struct parameters par;

    //definizione dei parametri
    par.boolean = 1;
    par.file_descriptor = fdr;
    

    //Inizializzo la pipe per la comunicazione del thread id
    int pipefd[2];
    pipe(pipefd);
    par.pipefdr = pipefd[0];

    //Disattivo il suo fd nel fdset
    FD_CLR(fdr,&fd);

    pthread_create(&id,NULL,init_client,(void*)&par);
    //aggiungo alla lista dei thread

    write(pipefd[1],&id,sizeof(pthread_t));
    close(pipefd[1]);

    create_thread(id,-1,7);

}

//ROUTINE DI CHIUSURA
void server_handler_closing_routine(){
    printf("Funzine non ancora definita\n");
    exit(EXIT_SUCCESS);
}

///////////////////////////////////////////////////////////////////////////////////////////
//FUNZIONI PER LA GESTIONE DELLA LISTA
//ADD REQUEST
//REMOVE REQUEST
//GET REQUEST

//Inserimento in testa
void add_request(struct request_list** head,struct client* requester, struct client* receiver, int id){
    
    //Creo la nuova request
    struct request* req = (struct request*)malloc(sizeof(struct request));
    if (!req) return;

    //Inserisco i valori
    req->requester = requester;
    req->receiver = receiver;
    req->request_id = id;

    //Creo il nodo
    struct request_list* new_node = (struct request_list*)malloc(sizeof(struct request_list));

    if (!new_node) {
        free(req);
        return;
    }

    new_node->req = req;
    new_node->next = *head;
    *head = new_node;
}

void remove_request(struct request_list** head,int id){
    //creo due puntatori temporanei
    struct request_list* current = *head, *prev = NULL;

    while(current != NULL){
        if(current->req->request_id == id){
            if(prev == NULL){
                //Il nodo è in testa
                *head = current->next;
            }else{
                prev->next = current->next;
            }
            free(current->req);
            free(current);
            return;
        }
        prev = current;
        current = current->next;
    }
}

struct request* get_request(struct request_list* head,int id){
    //Scorro la lista finchè non trovo un nodo con quel valore
    struct request_list* current = head;

    while(current != NULL){
        if(current->req->request_id == id){
            return current->req;
        }

        current = current->next;
    }

    return NULL;
}

void print_request_list(struct request_list* head){
    struct request_list* current = head;

    while(current != NULL){
        print_request(current->req);
        current = current->next;
    }
}
void print_request(struct request* req){
    printf("\n################\n");
    printf("Requester : %s\n",req->requester->username);
    printf("Receiver : %s\n",req->receiver->username);
    printf("Request id : %d\n\n",req->request_id);
}

/////////////////////////////////////////////////////////////////////////////////////
//API (Funzioni per inviare pacchetti per le richieste ai client da parte del server)
void send_connection_request(int fdw,char* buffer,int request_id){
    //Funzione per inviare una richiesta di connessione ali client

    //Inizializzazione delle variabili
    struct server_request_handler_pck pck;

    //Definizione del pacchetto
    pck.type = 0; //Connection request
    struct int_and_username* byte = (struct int_and_username*) pck.bytes; 
    byte->integer = request_id;
    memcpy(byte->buffer,buffer,USERNAME_LENGHT);
    pck.len = sizeof(struct int_and_username);
    

    //Invio del pacchetto
    if(write(fdw,&pck,sizeof(struct server_request_handler_pck)) < 0){
        perror("Write send_connection_request");
        exit(EXIT_FAILURE);
    }
}

void send_connection_accept(int fdw,char* buffer,int request_id){
    //Funzione per inviare una accettazione di connessione al client

    //Inizializzazione delle variabili
    struct server_request_handler_pck pck;

    //Definizione del pacchetto
    pck.type = 1; //Connection accept
    struct int_and_username* byte = (struct int_and_username*) pck.bytes; 
    byte->integer = request_id;
    memcpy(byte->buffer,buffer,USERNAME_LENGHT);
    pck.len = sizeof(struct int_and_username);

    //Invio il pacchetto
    if(write(fdw,&pck,sizeof(struct server_request_handler_pck)) < 0){
        perror("Write send_connection_request");
        exit(EXIT_FAILURE);
    }
}

void send_connection_refuse(int fdw,char* buffer,int request_id){
    //Funzione per inviare una accettazione di connessione al client

    //Inizializzazione delle variabili
    struct server_request_handler_pck pck;

    //Definizione del pacchetto
    pck.type = 2; //Connection refuse
    struct int_and_username* byte = (struct int_and_username*) pck.bytes; 
    byte->integer = request_id;
    memcpy(byte->buffer,buffer,USERNAME_LENGHT);
    pck.len = sizeof(struct int_and_username);

    //Invio il pacchetto
    if(write(fdw,&pck,sizeof(struct server_request_handler_pck)) < 0){
        perror("Write send_connection_request");
        exit(EXIT_FAILURE);
    }
}

void send_start_communication(int fdw){
    //Funzione per indicare che la comunicazione è pronta
    struct server_request_handler_pck pck;

    //Definizione del pacchetto;
    pck.type = 10;
    pck.len = 0;

    //Invio il pacchetto
    if(write(fdw,&pck,sizeof(struct server_request_handler_pck)) < 0){
        perror("Write send_connection_request");
        exit(EXIT_FAILURE);
    }
}

int get_new_id(){
    current_new_request_id++;
    return current_new_request_id - 1;
}
