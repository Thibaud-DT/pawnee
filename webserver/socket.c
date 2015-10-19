#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include "socket.h"

#define BUFFER_SIZE 4096

int socket_serveur;
int socket_client;
const char *message_bienvenue = "<html><head><title>Hello</title><body><h1>Bonjour, bienvenue sur mon serveur</h1></body></html>";
int optval = 1;

int creer_serveur(int port, char *document_root) {
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
	
	printf("SERVER CREATED ON PORT %d\n[%s]\n", port, document_root);

	while((socket_client = accept(socket_serveur, NULL, NULL))) {
		if(socket_client == -1)
			perror("ACCEPT SOCKET SERVEUR");
		printf("-----------------------------------------------\n");
		pid = fork();
		if(pid == 0){
			FILE *fp;
			char req[BUFFER_SIZE];
			int parse;
			int fd;
			http_request *request = (http_request *)malloc(sizeof(http_request));
			fp = fdopen(socket_client, "w+");
			fgets_or_exit(req, sizeof(req), fp);
			printf("Parsing request...\n");
			parse = parse_http_request(req, request);
			printf("METHOD :[%d], URI :[%s], VERSION :[%d.%d]\n", request->method, request->url, request->major_version, request->minor_version);
			skip_headers(fp);
			if(!parse)
				send_response(fp, 400, "Bad Request", "<h1>400: Bad Request</h1>");
			else if(request->method == HTTP_UNSUPPORTED)
				send_response(fp, 405, "Method Not Allowed", "<h1>405: Method Not Allowed</h1>");
			else if(request->major_version != 1 && (request->minor_version < 0 || request->minor_version > 1))
				send_response(fp, 505, "HTTP Version Not Supported", "<h1>505: HTTP Version Not Supported</h1>");
			else if((fd = check_and_open(request->url, document_root)) != -1) {
				char headers[1024];
				char mime_type[64];
				int fsize = get_file_size(fd);
				char buffer[fsize];
				get_mime_type(rewrite_url(request->url), mime_type);
				send_status(fp, 200, "OK");
				if(mime_type != NULL)
					sprintf(headers, "Content-Length: %d\r\nContent-Type: text/html\r\n\r\n", fsize);
				else
					sprintf(headers, "Content-Length: %d\r\nContent-Type: text/plain\r\n\r\n", fsize);
				fprintf(fp, headers);
				read(fd, buffer, fsize);
				close(fd);
				fprintf(fp, buffer);
				fprintf(fp, "\r\n"); 		
			}
			else
				send_response(fp, 404, "Not Found", "<h1>404: Not Found</h1>");
			return fclose(fp);
		}else if(pid == -1){
			perror("ERROR FORKING");
			return -1;
		}else{
	  		close(socket_client);
		}
	}
	return 0;
}

char *fgets_or_exit(char *buffer, int size, FILE *stream) {
	if(fgets(buffer, size, stream) == NULL) {
		printf("Connection was closed !\n");
		exit(1);
	}
}

void skip_headers(FILE *stream) {
	char req[BUFFER_SIZE];
	while(req[0] != '\r' && req[1] != '\n') fgets_or_exit(req, sizeof(req), stream);
}

void send_status(FILE *client, int code, const char *reason_phrase) {
	char response[256];
	sprintf(response, "HTTP/1.1 %d %s\r\n", code, reason_phrase);
	fprintf(client, response);
	printf("%d: %s\n", code, reason_phrase);
}

void send_response(FILE *client, int code, const char *reason_phrase, const char *message_body) {
	send_status(client, code, reason_phrase);
	if(message_body != NULL) {
		char content_length[256];
		char content_type[256];
		sprintf(content_length, "Content-Length: %d\r\n", strlen(message_body));
		sprintf(content_type, "Content-Type: text/html\r\n");
		fprintf(client, content_type);
		fprintf(client, content_length);
		fprintf(client, "\r\n");
		fprintf(client, message_body);
	}
	fprintf(client, "\r\n");
}

int get_mime_type(char *file, char *mime_type) {
	char *fextension;
	char fnull[256];
	char buff[256];
	FILE *fmime = fopen("/etc/mime.types", "r");
	fextension = get_filename_ext(file);
	printf("%s\n", fextension);
	while(fgets(buff, sizeof(buff), fmime) != NULL) {
		char extension[32];
		sscanf(buff, "%s\t\t\t\t%s\n", mime_type, extension);
		if(strcmp(fextension, extension) == 0)
			return 0;
		//printf("%s -> %s\n", mime_type, extension); 
	}
	return 1;
}

char *get_filename_ext(const char *filename) {
    char *dot = strrchr(filename, '.');
    if(!dot || dot == filename) return "";
    return dot + 1;
}

int parse_http_request(const char *request_line, http_request *request) {
	char *req;
	char *method;
	char *convert_int_error;
	char *uri;
	char *prot;
	const char *smajor_version;
	const char *sminor_version;
	int major_version;
	int minor_version;
	req = strdup(request_line);
	if((method = strtok (req," ")) == NULL) return 0;
	if((uri = strtok (NULL, " ")) == NULL) return 0;
	if((prot = strtok (NULL, "/")) == NULL) return 0;
	if((smajor_version = strtok (NULL, ".")) == NULL) return 0;
	if((sminor_version = strtok (NULL, "\r\n")) == NULL) return 0;
	if(strcmp(method,"GET") == 0)
		request->method = HTTP_GET;
	else
		request->method = HTTP_UNSUPPORTED;
	request->major_version = strtol(smajor_version, &convert_int_error, 10);
	if(*convert_int_error)
		request->major_version = -1;
	request->minor_version = strtol(sminor_version, &convert_int_error, 10);
	if(*convert_int_error)
		request->minor_version = -1;
	request->url = uri;
	return 1;
}

char *rewrite_url(char *url) {
	char *dupurl;
	char *rewrited_url;
	dupurl = strdup(url);
	if((rewrited_url = strtok(dupurl, "?")) == NULL)
		return url;
	else
		return rewrited_url;
}

int check_and_open(const char *url, const char *document_root) {
	char filename[256];
	struct stat s;
	int fd;
	sprintf(filename, "%s%s", document_root, url);
	if(stat(filename, &s) == 0) {
		if(s.st_mode & S_IFREG) {
			if((fd = open(filename, O_RDONLY)) != -1)
				return fd;
			else
				return -1; 	
		}
		else
			return -1;
	}
	else
		return -1;
}

int get_file_size(int fd) {
	struct stat s;
	fstat(fd, &s);
	return s.st_size;
}

int copy(int in, int out) {
	
}

void traitement_signal(int sig) {
	printf("-----------------------------------------------\n");
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
