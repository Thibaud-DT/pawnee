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

	while(1) {
		socket_client = accept(socket_serveur, NULL, NULL);
		printf("NOUVELLE CONNEXION %d\n", getpid());
		if(socket_client == -1) {
			perror("ACCEPT SOCKET SERVEUR");
		}
		pid = fork();
		if(pid == 0){
			char req[4096];
			read(socket_client, &req, 4096);
			printf("%s", req);
	  		write(socket_client, message_bienvenue, strlen(message_bienvenue));
			close(socket_client);
			return 0;
		}else if(pid == -1){
			perror("ERROR FORKING");
			return -1;
		} else {
	  		close(socket_client);
		}
	}
	return 0;
}

void traitement_signal(int sig) {
	wait();
	printf("FERMETURE CONNEXION\n");
}

int initialiser_signaux() {
	struct sigaction sa;
	sa.sa_handler = traitement_signal;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if(sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("FERMETURE CONNEXION");
	}
	/*if(signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
		perror("ERROR");
	}*/
	return 0;
}
