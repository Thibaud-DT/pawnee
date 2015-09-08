#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>

int socket_serveur;
int socket_client;

int creer_serveur(int port) {
	struct sockaddr_in saddr;
	socket_serveur = socket(AF_INET, SOCK_STREAM, 0);
	if(socket_serveur == -1) {
		perror("SOCKET SERVEUR");
		return -1;
	}
	saddr.sin_family = AF_INET;
	saddr.sin_port = htons(8080);
	saddr.sin_addr.s_addr = INADDR_ANY;
	if(bind(socket_serveur, saddr, sizeof(saddr)) == -1) {
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
	write(socket_client, message_bienvenue, strlen(message_bienvenue));
	return 0;
}
