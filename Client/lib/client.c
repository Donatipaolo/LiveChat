#include "client.h"
#include <sys/select.h>
#include <sys/socket.h>


//Function
void change_username(int sockfd){

    //Inizializzazione delle variabili
    char buffer[USERNAME_LENGHT];
    memset(buffer,0,USERNAME_LENGHT);
    int result;

    while(1){
        
        printf("Inserisci il tuo username : ");
        fflush(stdout);
        
        // Leggo l'input in modo sicuro
        if (safe_fgets(buffer, USERNAME_LENGHT - 1) == 0) {
            printf("Errore nella lettura dell'input\n");
            exit(1);
        }
        
        // Assicuro che la stringa termini con \0
        buffer[USERNAME_LENGHT-1] = '\0';
        strip_newline(buffer);
        
        //invio il buffer al server
        if(send(sockfd, buffer, USERNAME_LENGHT, 0) < 0){
            perror("Write");
            exit(1);
        }

        printf("In attesa di una risposta dal server...\n\n");
        //Aspetto la risposta
        read(sockfd, &result,sizeof(int));

        if(!result){
            printf("Username registrato correttamente\n");
            return;
        }

        printf("Username gia' in uso, Inserirne un altro per continuare\n");
    }
    
}

void update_client(int sockfd){

    send_client_update(sockfd);
    int dim;
    //i primi 4 byte sono il numero di di client connessi i byte successivi gli username per ogni client
    read(sockfd,&dim,sizeof(int));

    printf("Dimensione della lista : %d\n",dim);

    if(dim > clients_dim){
        printf("Change client dim da %d a %d\n",clients_dim,dim);
        change_clients_dim(dim+5);
        //Aggiungo altro spazio
        clients_dim = dim+5;
    }

    //inserisco gli username nel clients
    for(int i = 0; i < dim; i++){
        //leggo dal buffer direttamente nello spazio allocato
        read(sockfd,clients[i],sizeof(struct client_info));
    }

    clients_current = dim;
    
    system("clear");
    print_client(clients,clients_current);
    fflush(stdout);
}

//Per la gesitone delle richieste
void request_handler(int sockfd){

    //Creo due setfd
    fd_set fd,tempfd;
    FD_ZERO(&fd);
    FD_SET(fileno(stdin), &fd);
    FD_SET(sockfd, &fd);

    //Per leggere velocemente
    int fds[2] = {sockfd,fileno(stdin)};
    int exit_v;
    int scelta;

    struct timeval timeout;

    while(1){
        tempfd = fd;

        timeout.tv_sec = 10;
        timeout.tv_usec = 0;

        //Ci troviamo nel menu principale quindi stampo a schermo i client
        update_client(sockfd);
        //Stampo a schermo le scelte possibili
        printf("Inserisci la tua scelta :\n");
        printf("0) Per Inviare una richiesta di comunicazione\n");
        printf("1) Per aggiornare la lista dei client\n");
        printf("2) Per cambiare il tuo username\n");
        printf("3) Per uscire dal programma\n");
        printf("> ");
        fflush(stdout);
        
        if(select(sockfd+1,&tempfd,NULL,NULL,&timeout) < 0){
            perror("Select");
            //Comunico al server che esco
            send_client_exit(sockfd);
            exit(1);
        }
        exit_v = 0;
        for(int i =0; i < 2; i++){
            if(FD_ISSET(fds[i],&tempfd)){
                if(i == 1){
                    scanf("%d",&scelta);
                    clear_input_buffer();
                    
                    if(!server_clear(sockfd,0,5000)){
                        switch(scelta){
                            case 0:
                                scelta = choose_client(sockfd);
                                if(scelta == -1){
                                    handle_server_request(sockfd);
                                    break;
                                }
                                if(scelta == -2){
                                    break;
                                }

                                handle_connection_request(sockfd,scelta-1);
                                break;
                            case 1:
                                break;
                            case 2:
                                send_change_username(sockfd);
                                system("clear");
                                change_username(sockfd);
                                break;
                            case 3:
                                send_client_exit(sockfd);
                                exit(0);
                            default:
                                printf("Input non valido\n");
                                wait_key_press();
                                break;
                        }
                    }else{
                        handle_server_request(sockfd);
                        exit_v = 1;
                    }

                }
                else{
                    handle_server_request(sockfd);
                    exit_v = 1;
                }
            }
            if(exit_v) break;
        }
        system("clear");
    }
}

int server_clear(int sockfd,int seconds,int microseconds){
    fd_set tempfd;

    FD_ZERO(&tempfd);
    FD_SET(sockfd,&tempfd);

    struct timeval timeout;
    timeout.tv_sec = seconds;
    timeout.tv_usec = microseconds;

    int result = select(sockfd+1,&tempfd,NULL,NULL,&timeout);

    if(result != -1 && FD_ISSET(sockfd,&tempfd)){
        return 1;
    }
    return 0;
}

int choose_client(int sockfd){
    clear_last_lines(4);
    printf("Inserisci con chi vuoi comunicare : ");
    fflush(stdout);
    int scelta;

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(fileno(stdin),&fd);
    FD_SET(sockfd,&fd);

    if(select(sockfd+1,&fd,NULL,NULL,NULL)< 0){
        perror("Select");
        //Comunico al server che voglio uscire
        send_client_exit(sockfd);
        exit(1);
    }

    if(FD_ISSET(sockfd,&fd)){
        return -1;
    }
    else if(FD_ISSET(fileno(stdin), &fd)) {
        char buffer[32];
        if (safe_fgets(buffer, sizeof(buffer)) == 0) {
            printf("Errore nella lettura dell'input.\n");
            return -2;
        }
    
        scelta = atoi(buffer);  // Converte la stringa in intero
    
        if (scelta > clients_current || scelta < 0) {
            printf("Scelta non valida\n");
            return -2;
        }
    
        return scelta;
    }
    return -1;
}

void handle_server_request(int sockfd){
    //Funzione che gestisce le richieste che arrivano dal server
    struct server_request_handler_pck pck;

    //leggo dal server quello che c'è da leggere
    int bytes = read(sockfd,&pck,sizeof(struct server_request_handler_pck));

    if(bytes == 0 || bytes == -1){
        system("clear");
        printf("Server non è più accessibile\n");
        wait_key_press();
        exit(0);
    }

    //Controllo quello che ho letto
    switch(pck.type){
        case 0:
            handle_connection_request_ricezione(sockfd,&pck);
            break;
        default:
            break;
    }
}

void handle_connection_request(int sockfd,int index){
    //Inizializzazione delle variabili
    struct server_request_handler_pck pck;
    
    //Pulisco lo schermo
    system("clear");

    //Invio la richiesta
    send_connection_request(sockfd,clients[index]->buffer);

    //mando a la parte grafica
    print_communication_wait(clients[index]->buffer);

    while(1){

        //Leggo il risultato
        read(sockfd,&pck,sizeof(struct server_request_handler_pck));

        switch(pck.type){
            case 0:
                send_connection_refuse(sockfd,&pck);
                break;
            case 1:
                struct int_and_username *v = (struct int_and_username *)pck.bytes;
                communication(sockfd,v->buffer);
                return;
            case 2:
                print_communication_refuse(&pck,(char*)pck.bytes + sizeof(int));
                wait_key_press();
                return;
            default:
                break;
        }
    }
}

void handle_connection_request_ricezione(int sockfd, struct server_request_handler_pck* pck){
    //Inizializzazione delle variabili
    struct int_and_username *value = (struct int_and_username *)pck->bytes;
    int result;
    char response[10];

    //pulisco lo schermo
    system("clear");

    //Mostro a schermo la richiesta
    print_communication_request((char*)value->buffer);

    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(fileno(stdin),&fd);

    struct timeval timeout;
    timeout.tv_sec = 10;
    timeout.tv_usec = 0;

    result = select(fileno(stdin)+1,&fd,NULL,NULL,&timeout);

    if(FD_ISSET(fileno(stdin),&fd) && result > 0){
        //Leggo dal buffer
        safe_fgets(response,sizeof(char)*10);
        char *newline = strchr(response, '\n');
        if (newline) *newline = '\0';//Toglo il new line

        if((strcmp(response,"s") == 0 || strcmp(response,"S") == 0) && result != 0){
            send_connection_accept(sockfd,pck);
            communication(sockfd,value->buffer);
            return;
        }
    }

    send_connection_refuse(sockfd,pck);
}

void communication(int sockfd,char buffer[USERNAME_LENGHT]){
    //Inizializzazione della comunicazione
    char msg[MAX_MSG_LENGHT];
    struct communication_handler_pck pck;

    //pulisco lo schermo
    system("clear");
    printf("Inizializzazione della comunicazione...\n");
    wait_for_start_communication(sockfd);

    system("clear");

    //Inizializzazione della schermata
    print_init_communication(buffer);

    //Inizializzaizone deli fd
    fd_set fd,tempfd;
    FD_ZERO(&fd);
    FD_SET(fileno(stdin),&fd);
    FD_SET(sockfd,&fd);

    while(1){
        tempfd = fd;


        if(select(sockfd+1,&tempfd,NULL,NULL,NULL) < 0){
            perror("Select");
            send_communication_exit(sockfd,&pck);
            exit(1);
        }


        if(FD_ISSET(fileno(stdin),&tempfd)){
            //leggo dallo stdin
            safe_fgets(msg,MAX_MSG_LENGHT);
            msg[MAX_MSG_LENGHT-1] = '\0';
            msg[strcspn(msg, "\n")] = '\0';
            
            //controllo se è un segnale di chiusura
            if(strcmp(msg,"/exit") == 0){
                send_communication_exit(sockfd,&pck);
                wait_for_communication_exit(sockfd,&pck);
                return;
            }

            send_communication_msg(sockfd,msg,&pck);
            print_my_msg(msg);
        }

        if(FD_ISSET(sockfd,&tempfd)){

            //Leggo il pacchetto:
            int bytes = read(sockfd,&pck,sizeof(struct communication_handler_pck));

            if(bytes == -1 || bytes == 0){
                //Indico di chiudere la comunicazione 
                system("clear");
                printf("Il server è irraggiungibile\n");
                exit(1);
            }

            //controllo i risultati
            switch(pck.type){
                case 0:
                    print_other_msg(buffer,(char*)pck.bytes);
                    break;
                case 1:
                    system("clear");
                    printf("%s si è disconnesso\n",buffer);
                    wait_key_press();
                    return;
                default:
                    break;
            }
        }
    }



}

void wait_for_start_communication(int sockfd){

    struct server_request_handler_pck pck;

    while(1){
        read(sockfd,&pck,sizeof(struct server_request_handler_pck));

        if(pck.type == 10) return;
    }
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//Funzioni per inviare messaggio al server

void send_connection_request(int sockfd,char buffer[USERNAME_LENGHT]){
    //Funzione che mi permette di inviare al server una connection request
    //Creo un pacchetto di tipo 0 e comne parametri gli passo solo lo username
    struct server_request_handler_pck pck;

    //Definisco il pacchetto
    pck.type = 0;
    pck.len = USERNAME_LENGHT;
    memcpy(pck.bytes,buffer,USERNAME_LENGHT);

    //invio il pacchetto
    write(sockfd,&pck,sizeof(struct server_request_handler_pck));
}

void send_connection_accept(int sockfd,struct server_request_handler_pck *pck){
    //Accetto la richiesta di connessione
    //pck è il pacchetto che mi è stato inviato, basta che modifico il tipo:
    pck->type = 1;

    //Invio il pacchetto
    write(sockfd,pck,sizeof(struct server_request_handler_pck));
}

void send_connection_refuse(int sockfd,struct server_request_handler_pck *pck){
    //rifiuto la richiesta di connessione
    //pck è il pacchetto che mi è stato inviato, basta che modifico il tipo:
    pck->type = 2;

    //Invio il pacchetto
    write(sockfd,pck,sizeof(struct server_request_handler_pck));
}

void send_client_update(int sockfd){
    //Invio una richiesta di client update
    struct server_request_handler_pck pck;

    //Definizione del pacchetto
    pck.type = 5;
    pck.len = 0;

    //Invio il pacchetto
    write(sockfd,&pck,sizeof(struct server_request_handler_pck));
}

void send_client_exit(int sockfd){
    //Dico al server che esco
    struct server_request_handler_pck pck;

    //Definizione del pacchetto
    pck.type = 4;
    pck.len = 0;

    //Invio il pacchetto
    write(sockfd,&pck,sizeof(struct server_request_handler_pck));
}

void send_communication_msg(int sockfd, char buffer[MAX_MSG_LENGHT],struct communication_handler_pck* pck){
    //Invio un pacchetto di tipo 0
    //Definzione del pacchetto
    pck->type = 0;
    memcpy(pck->bytes,buffer,MAX_MSG_LENGHT);
    pck->len = MAX_MSG_LENGHT;

    //Invio il pacchetto
    write(sockfd,pck,sizeof(struct communication_handler_pck));
}

void send_communication_exit(int sockfd,struct communication_handler_pck* pck){
    //Invio un pacchetto di tipo 1
    //Definizione pacchetto
    pck->type = 1;
    pck->len = 0;

    //Invio il pacchetto
    write(sockfd,pck,sizeof(struct communication_handler_pck));
}

void wait_for_communication_exit(int sockfd,struct communication_handler_pck* pck){
    while(1){
        read(sockfd,pck,sizeof(struct communication_handler_pck));

        if(pck->type == 1){
            return;
        }
    }
}

void send_change_username(int sockfd){
    //Invio un pacchetto di tipo 6
    struct server_request_handler_pck pck;
    
    //Definizione del pacchetto
    pck.type = 6;
    pck.len = 0;

    //Invio il pacchetto
    write(sockfd,&pck,sizeof(struct server_request_handler_pck));
}

///////////////////////////////////////////////////////////////////////////////////////////////////////
//Funzioni per la parte grafica

void print_client(struct client_info **info, int n_client) {
    system("clear");
    printf("==========================================\n");
    printf("           CLIENT CONNESSI ATTUALI        \n");
    printf("==========================================\n\n");

    if (n_client > 0) {
        printf(" #   | Status   | Username\n");
        printf("-----+----------+--------------------------\n");

        for (int i = 0; i < n_client; i++) {
            const char *state_str;
            const char *color;

            if (info[i]->status == 0) {
                state_str = "free";
                color = "\033[0;32m"; // Verde
            } else {
                state_str = "occupied";
                color = "\033[0;33m"; // Giallo
            }

            printf(" %2d  | %s%-8s\033[0m | %s\n", i + 1, color, state_str, info[i]->buffer);
        }

        printf("\n==========================================\n");
        printf(" Totale client connessi: %d\n", n_client);
        printf("==========================================\n");
    } else {
        printf("In questo momento non sono presenti altri client...\nRiprova più tardi.\n\n");
    }
}

void print_communication_wait(char buffer[USERNAME_LENGHT]){
    system("clear");
    printf("\n==========================================\n");
    printf("  RICHIESTA DI COMUNICAZIONE INVIATA A:  \n");
    printf("           %s\n", buffer);
    printf("==========================================\n");
    printf("\nIn attesa della risposta...\n");
}

void print_communication_accept(char buffer[USERNAME_LENGHT]){
    printf("\n\n%s ha accettato la comunicazione!\n", buffer);
}

void print_communication_refuse(struct server_request_handler_pck *pck,char buffer[USERNAME_LENGHT]){
    //Controllo il tipo di pacchetto
    int status = *((int*)pck->bytes);

    system("clear");

    if(status == -1){
        printf("Client non trovato, prova a riaggiornare la lista dei client\n");
    }
    
    else if(status == -2){
        printf("Il client selezionato è in un altra comunicazione, riprovare più tardi\n");
    }

    else{
        printf("%s ha rifiutato la comunicazione.\n", buffer);
    }
    fflush(stdout);
}

void print_communication_request(char buffer[USERNAME_LENGHT]){
    system("clear");
    printf("==========================================\n");
    printf("   RICHIESTA DI COMUNICAZIONE IN ARRIVO   \n");
    printf("==========================================\n");
    printf("\nL'utente %s vuole comunicare con te.\n", buffer);
    printf("Vuoi accettare? [s/n] (hai 10 secondi): ");
    fflush(stdout);
}

void print_init_communication(char buffer[USERNAME_LENGHT]){
    printf("==========================================\n");
    printf("            CHAT CONNESSA CON:           \n");
    printf("               %s\n",buffer);
    printf("==========================================\n");
    printf("(/exit)> ");
    fflush(stdout);
}

void print_my_msg(char buffer[MAX_MSG_LENGHT]){
    //elimino la righa corrente
    clear_last_lines(1);
    printf("\033[1;32m");
    printf("[Tu]      : %s\n",buffer);
    printf("\033[0m");  // Reset dei colori
    printf("(/exit)> ");
    fflush(stdout);
}

void print_other_msg(char usr[USERNAME_LENGHT],char buffer[MAX_MSG_LENGHT]){
    printf("\33[2K");
    printf("\033[1;34m");
    printf("\r[%s]      : %s\n",usr,buffer);
    printf("\033[0m");  // Reset dei colori
    printf("(/exit)> ");
    fflush(stdout);
}


////////////////////////////////////////////////////////////////////////////////////
//Altre funzioni

void wait_key_press() {
    printf("Premi Invio per continuare...");
    while (getchar() != '\n'); // legge tutto fino a \n
}

void change_clients_dim(int dim){

    //Cambio la dimensione dei clients
    //libero tutto lo spazio
    for(int i = 0; i < clients_dim; i++){
        //per ognuno elimino il nome
        free(clients[i]);
    }

    free(clients);

    clients = malloc(sizeof(char*)*dim);

    for(int i = 0; i < dim; i++){
        clients[i] = malloc(sizeof(struct client_info));
    }
}

void clear_input_buffer() {
    int ch = 0;
    while (ch != '\n' && ch != EOF) {
        ch = getchar();  // Consuma i caratteri rimanenti nel buffer
    }
}

void clear_last_lines(int n) {
    for (int i = 0; i < n; ++i) {
        printf("\33[1A");     // Spostati una riga sopra
        printf("\33[2K");     // Cancella l'intera riga
    }
    fflush(stdout);           // Forza output
}

void strip_newline(char *str) {
    if (str == NULL) return;

    char *newline = strchr(str, '\n');
    if (newline) {
        *newline = '\0';
    }
}


//Per prendere in input i valori in maniera sicura
int safe_fgets(char *buffer,ssize_t dim){
    if (fgets(buffer, dim, stdin) != NULL) {
        // Controllo se il buffer contiene '\n'
        if (strchr(buffer, '\n') == NULL) {
            // '\n' non trovato: la riga era troppo lunga, pulisco il buffer
            clear_input_buffer();
        }
        return 1;
    }
    return 0;
}