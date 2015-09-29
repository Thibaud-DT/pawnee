#ifndef __SOCKET_H__
#define __SOCKET_H__

int creer_serveur(int port);
int initialiser_signaux();
void response_200(FILE *fp);
void response_400(FILE *fp);
void response_404(FILE *fp);

#endif
