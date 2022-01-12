#ifndef __CLIENT_H
#define __CLIENT_H


#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdbool.h>

#define PORT 6000
#define MAX_BUFFER 1000

int ouvrirUneConnexionTcp();
void lireMessage(char tampon[]);
int testQuitter(char tampon[]);
int checkRecurrence(char lettre, char* lettreUtilise);

void affPendu(int nbErreur);

#endif