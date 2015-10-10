#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include "socket.h"

#define BAD_UTILISATION "/!\\ Mauvaise utilisation\n	-> webever [PORT] [DOCUMENT_ROOT]\n" 
int main(int argc, char **argv){
	struct stat s;
	if(argc != 3) {
		printf("%s", BAD_UTILISATION);
		return -1;
	}
	if(stat(argv[2], &s) == 0) {
		if(s.st_mode & S_IFDIR) {
			if(creer_serveur(atoi(argv[1]), argv[2]) == -1){
				perror("CREATION SERVEUR");
    				return -1;
  			}
		}
		else {
			printf("%s[%s]  n'est pas un dossier\n", BAD_UTILISATION, argv[2]);
		}
	}
	else {
		printf("%s", BAD_UTILISATION);
		return -1;
	}
	return 0;
}
