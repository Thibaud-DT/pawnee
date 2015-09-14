#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <unistd.h>

int socket_serveur;
int socket_client;
const char *message_bienvenue = "Bonjour, bienvenue sur mon serveur \n";
int optval = 1;

int creer_serveur(int port) {
	struct sockaddr_in saddr;
	pid_t pid;
	
	socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
	initialiser_signaux();
	if(socket_serveur == -1) {
		perror("SOCKET SERVEUR");
		return -1;
	}
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8080);
	saddr.sin_addr.s_addr = INADDR_ANY;

	if(setsockopt(socket_serveur, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(int)) == -1) {
		perror("SET SO_REUSEADDR OPTION");
		return -1;
	}

	if(bind(socket_serveur, (struct sockaddr*)&saddr, sizeof(saddr)) == -1) {
		perror("BIND SOCKET SERVEUR");
		return -1;
	}

	if(listen(socket_serveur, 10) == -1) {
		perror("LISTEN SOCKET SERVEUR");
		return -1;
	}

	socket_client = accept(socket_serveur, NULL, NULL);
	if(socket_client == -1) {
		perror("ACCEPT SOCKET SERVEUR");
	}
	pid = fork();
	if(pid ==0){
	  write(socket_client, message_bienvenue, strlen(message_bienvenue));

	  /* traitement du client */

	}else{
	  	  
	}


	return 0;
}

int initialiser_signaux() {
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR){
		perror("SIGNAL");	
		return -1;
	}
	return 0;
}
