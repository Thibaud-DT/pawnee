#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdlib.h>
#include "socket.h"

#define BUFFER_SIZE 4096


#define E_400 "HTTP/1.1 400 Bad Request\r\nConnection: close\r\nContent-Length: 17\r\n\r\n400 Bad request\r\n"
#define E_404 "HTTP/1.1 404 Not Found\r\nConnection: close\r\nContent-Length: 13\r\n\r\n404 Not Found\r\n"
#define OK "HTTP/1.1 200 OK"

int socket_serveur;
int socket_client;
const char *message_bienvenue = "<h1>Bonjour, bienvenue sur mon serveur</h1>";
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
	saddr.sin_port = htons(port);
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

	while((socket_client = accept(socket_serveur, NULL, NULL))) {
		if(socket_client == -1)
			perror("ACCEPT SOCKET SERVEUR");
		pid = fork();
		if(pid == 0){
			FILE *fp;
			char req[BUFFER_SIZE];
			int nbbytes = 0;
			char *method, *uri, *version;
			fp = fdopen(socket_client, "w+");
			if(fgets(req, sizeof(req), fp) != NULL){
				method = strtok (req," ");
				uri = strtok (NULL, " ");
				version = strtok (NULL, " ");
				if(strcmp(method,"GET") == 0 && (strncmp(version,"HTTP/1.0",8) == 0 || strncmp(version,"HTTP/1.1",8) == 0)){
					printf("METHOD :[%s]/ URI :[%s]/ VERSION :[%s]", method,uri,version);
					while(fgets(req, sizeof(req), fp) != NULL && req[0] != '\n' && req[0] != '\r');

					if(strcmp(uri, "/") == 0 || strcmp(uri, "") == 0)
						response_200(fp);			
					else
						response_404(fp);	
				}
				else
					response_400(fp);
			}
			fclose(fp);
			return 0;
		}else if(pid == -1){
			perror("ERROR FORKING");
			return -1;
		}else{
	  		close(socket_client);
		}
	}
	return 0;
}

void response_400(FILE *fp){
	fprintf(fp,E_400);
}

void response_404(FILE *fp) {
	fprintf(fp,E_404);
}

void response_200(FILE *fp){
	char response[1024];
	char itoa[10];

	strcpy(response, "HTTP/1.1 200 OK\r\nContent-Length: ");
	sprintf(itoa, "%d", strlen(message_bienvenue));
	strcat(response, itoa);
	strcat(response, "\r\n");
	strcat(response, "Content-Type: text/html\r\n");
	strcat(response, "\r\n");
	strcat(response, message_bienvenue);
	
	strcat(response, "\r\n");

	printf("%s",response);

	fprintf(fp, response);
}

void traitement_signal(int sig) {
	printf("FERMETURE CONNEXION\n");
	waitpid(-1, &sig, WNOHANG);
}

int initialiser_signaux() {
	struct sigaction sa;
	sa.sa_handler = traitement_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("ERROR SIGCHLD HANDLER");
	}
	if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		perror("ERROR SIGPIPE");
	}
	return 0;
}
