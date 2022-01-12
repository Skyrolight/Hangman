#ifndef __SERVEUR_H
#define __SERVEUR_H

#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <stdbool.h>


#define PORT 6000
#define MAX_BUFFER 1000
#define MAX_CLIENTS 10
#define LIST_SIZE 4382

const char *EXIT = "exit";

int ouvrirUneSocketAttente();
void motChoix(int ligneNb, FILE *fp, char mot[]);
char* remplirBlanc(char lettre, char* mot, char motCache[]);
int checkRecurrence(char lettre, char* lettreUtilise);

void affPendu(int nbErreur);

void lireMessage(char tampon[]);
int testQuitter(char tampon[]);


#endif