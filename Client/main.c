#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/socket.h> 
#include <netinet/in.h>
#include <arpa/inet.h>
#include "./lib/client.h"
#define PORT 5700
#define MAX_SIZE 1024
char IP[] =  "127.0.0.1";

char **clients = NULL;
int clients_dim = 0;
int clients_current = 0;

int initSocket(int domain, int type, int protocol, in_addr_t IP, int port,struct sockaddr_in** addr_r);
in_addr_t ip_to_addr_t(char *ip);

int main(){

    //Inizializzo le variabili
	struct sockaddr_in* addr;
	int status,sockfd;

	//Inizializzo il socket
	sockfd = initSocket(AF_INET,SOCK_STREAM,0,ip_to_addr_t(IP),PORT,&addr);

	//Provo a connettermi
	if((status = connect(sockfd,(struct sockaddr*)addr,sizeof(struct sockaddr_in))) < 0){
		perror("Connect");
		exit(EXIT_FAILURE);
	}

    change_username(sockfd);
	printf("sto entrando in request handler\n");
	request_handler(sockfd);
}


int initSocket(int domain, int type, int protocol, in_addr_t IP, int port,struct sockaddr_in** addr_r){

	//creazione della socket e controllo errori
	int fd = socket(domain, type, protocol);

	//alloco dinamicamente
	struct sockaddr_in* addr;
	addr = (struct sockaddr_in*)malloc(sizeof(struct sockaddr_in));


	if(fd < 0){
		perror("Socket");
		exit(EXIT_FAILURE);
	}

	//Imposto i valori nella struct
	addr->sin_family = domain;
	addr->sin_addr.s_addr = IP;
	addr->sin_port = htons(port);

	*addr_r = addr;
	return fd;
}

in_addr_t ip_to_addr_t(char *ip){

	struct in_addr addr;

    if (inet_pton(AF_INET, ip, &addr) != 1) {
        
        return 0;
    }
    
    return addr.s_addr;
}
