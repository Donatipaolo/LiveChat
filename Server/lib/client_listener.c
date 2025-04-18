#include "client_listener.h"
#include "client_handler.h"
#include "mthread_handler.h"

int initSocket(int domain, int type, int protocol, int IP, int port,struct sockaddr_in** addr_r){

    int opt = 1;

	//creazione della socket e controllo errori
	int fd = socket(domain, type, protocol);

	//alloco dinamicamente
	struct sockaddr_in* addr;
	addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));


	if(fd < 0){
		perror("Socket");
		exit(EXIT_FAILURE);
	}

    // Imposta SO_REUSEADDR
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0) {
        perror("setsockopt failed");
        close(fd);
        exit(EXIT_FAILURE);
    }

	//Imposto i valori nella struct
	addr->sin_family = domain;
	addr->sin_addr.s_addr = IP;
	addr->sin_port = htons(port);

	//binding dei valori
	if(bind(fd,(struct sockaddr*)addr,sizeof(struct sockaddr_in)) < 0){
		perror("Binding");
		exit(EXIT_FAILURE);
	}

	*addr_r = addr;
	return fd;
}

void* client_listener(void* arg){

    //Inizializzo le variabili
	int sockfd,clientfd;
	socklen_t addrlen = sizeof(struct sockaddr_in);
	struct sockaddr_in* addr;

    //Inizializzo il socket
    sockfd = initSocket(AF_INET,SOCK_STREAM,0,INADDR_ANY,PORT,&addr);

    //Parametri
    pthread_t id;
    struct parameters* par = malloc(sizeof(struct parameters));

    //Pipe per la comunicazione
    int pipefd[2];
    

    //apriamo la comunicazione in lettura
	while(1){
        
        //metto il server in ascolto e controllo un possibile errore
		if(listen(sockfd, 3) < 0){
            perror("Listen");
			exit(EXIT_FAILURE);
		}
        
		//accetto la richiesta di connessione e controllo un possibile errore
		if((clientfd = accept(sockfd,(struct sockaddr*)addr,&addrlen)) < 0){
            perror("Accept");
			exit(EXIT_FAILURE);
		}
        
		//creo un processo figlio per gestire la comunicazione con il client
        pipe(pipefd);

        //Imposto i parametri
        par->file_descriptor = clientfd;
        par->boolean = 0; //Creazione di un nuovo utente
        par->pipefdr = pipefd[0];
        
        //*((int*)(parameters + sizeof(int))) = id;
        // printf("Client.\nsockfd : %d\npipefd : %d\n\n",par->file_descriptor,par->pipefdr);

		pthread_create(&id,NULL,init_client,(void*)par);
        //aggiungo alla lista dei thread

        write(pipefd[1],&id,sizeof(pthread_t));
        close(pipefd[1]);


        if(fd_max < par->file_descriptor)
            fd_max = par->file_descriptor;

        create_thread(id,-1,7);
	}

}

void* init_client(void *arg){
    //Prendo i parametri
    struct parameters par = *(struct parameters*)(arg);
    // printf("Inizializzazione del client.\nsockfd : %d\npipefd : %d\n\n",par.file_descriptor,par.pipefdr);
    //Inizializzo il buffer
    char buffer[USERNAME_LENGHT];
    memset(buffer,0,USERNAME_LENGHT);

    //Trovo il mio thread_id
    pthread_t id;
    read(par.pipefdr,&id,sizeof(pthread_t));  
    close(par.pipefdr);

    //Inizializzo il result value
    int result;

    // //TEST
    // ssize_t bytesRead = read(par.file_descriptor, buffer, 12);
    // if (bytesRead < 0) {
    //     perror("read error");
    // } else {
    //     printf("Stringa letta: %s\n", buffer);
    // }

    while(1){
        
        //read_form_fd(par.file_descriptor);
        //leggo dal client l'username
        if(recv(par.file_descriptor, buffer, USERNAME_LENGHT, 0) < 0){
            perror("Read");
            exit(1);
        }
        buffer[USERNAME_LENGHT-1] = '\0';

        //comunico al client_handler di controllare se esiste un client con quel'username
        result = exists_client(par.file_descriptor);
    
        //Invio il risultato al client
        write(par.file_descriptor,&result,sizeof(int));

        //Controllo il risultato
        if(!result){
            //Se non esiste il client allora posso crearne uno nuovo con quel nome
            if(!par.boolean){
                create_client(par.file_descriptor,buffer);
            }
            else{
                change_username_client(par.file_descriptor,buffer);
            }
            //Finche non viene elaborata dal thread handler la richiesta fatta nella funzione precedente aspetto
            while(get_thread(id) == NULL){
                usleep(5000);
            }
            
            //A questo punto lo tolgo dalla lista
            remove_thread(id);

            //Anche se questo passaggio può sembrare inutile, e necessario salvare ogni thread nella lista.
            //Questo per una miglior gestione e controllo dei thread, permettendo la loro terminazione e accessibilità in ogni momento

            //Aggiungo FD
            FD_SET(par.file_descriptor,&fd);

            if(par.file_descriptor > fd_max)
                fd_max = par.file_descriptor;

            //chiudo il thread
            pthread_exit(NULL);
        }
    }  
}

ssize_t read_exact(int fd, void *buf, size_t count) {
    size_t total_read = 0;
    char *ptr = (char *)buf;

    while (total_read < count) {
        ssize_t bytes_read = read(fd, ptr + total_read, count - total_read);
        if (bytes_read <= 0) {
            return -1; // Errore o EOF
        }
        total_read += bytes_read;
    }

    return total_read; // Ora `buf` contiene esattamente `count` byte validi
}

void read_form_fd(int sfd){
    fd_set fd;
    FD_ZERO(&fd);
    FD_SET(sfd,&fd);

    select(sfd+1,&fd,NULL,NULL,NULL);
}