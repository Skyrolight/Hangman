#include "serveur.h"

int main(int argc, char const *argv[]) {
    FILE* fp = NULL;
    int fdSocketAttente, fdSocketCommunication, nbRecu, tailleMot, ligneNb, erreur = 0, trouve = 1, resultat = 0;
    struct sockaddr_in coordonneesAppelant;
    struct sockaddr_in coordonneesServeur;
    char tampon[MAX_BUFFER], motCache[50] = {"\0"};
    char mot[25], lettreUtilise[25] = { '\0' }, lettre, firstLettre;

    int longueurAdresse;
    int pid;

    // connexion serveur vers client
    fdSocketAttente = socket(PF_INET, SOCK_STREAM, 0);
    fdSocketAttente = ouvrirUneSocketAttente();
    socklen_t tailleCoord = sizeof(coordonneesAppelant);

    int nbClients = 0;

    while (nbClients < MAX_CLIENTS) {
        if ((fdSocketCommunication = accept(fdSocketAttente, (struct sockaddr *) &coordonneesAppelant,
                                            &tailleCoord)) == -1) {
            printf("erreur de accept\n");
            exit(EXIT_FAILURE);
        }

        printf("Client connecté - %s:%d\n",
               inet_ntoa(coordonneesAppelant.sin_addr),
               ntohs(coordonneesAppelant.sin_port));

        if ((pid = fork()) == 0) {
            close(fdSocketAttente);

            while (1) {
                // ouverture du fichier contenant les mots
                fp = fopen("mots_pendu.txt" , "r");
                if (fp == NULL)
                {
                    printf("Erreur ! Fichier inconnu");
                    return EXIT_FAILURE;
                }
                
                // choix du mot à trouver
                srand(time(NULL));
                ligneNb = rand()%(LIST_SIZE - 1);
                motChoix(ligneNb, fp, mot);
                firstLettre = mot[0];
                lettreUtilise[0] = firstLettre;
                
                //envoie du mot à trouver
                strcpy(tampon, mot);
                send(fdSocketCommunication, tampon, strlen(tampon), 0);

                /* verification mot recurrent */

                //reçoit la lettre
                nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);
                if (nbRecu > 0)
                {
                    tampon[nbRecu] = 0;
                    lettre = tampon[0];

                    int nbRecurrence = checkRecurrence(lettre, lettreUtilise);
                    if (nbRecurrence > 0 )
                    {
                        // envoie message si recurrence
                        tampon[0] = nbRecurrence;
                        send(fdSocketCommunication, tampon, strlen(tampon), 0);
                        break;
                    } else {

                    resultat = checkRecurrence(lettre, mot);
                        {
                            static int i = 0;
                            lettreUtilise[i] = lettre;
                            i++;
                            if (resultat == 0)
                            {
                                tampon[0] = -1;
                                send(fdSocketCommunication, tampon, strlen(tampon), 0);
                            } else {
                                trouve = trouve + resultat;
                                tampon[0] = 1;
                                send(fdSocketCommunication, tampon, strlen(tampon), 0);
                            }
                        }
                    }
                }
                

                /*

                printf("Saisir un message à envoyer :\n");
                fgets(tampon, MAX_BUFFER, stdin);
*/
                // on attend le message du client
                // la fonction recv est bloquante
                nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);

                if (testQuitter(tampon)) {
                    send(fdSocketCommunication, tampon, strlen(tampon), 0);
                    break; // on quitte la boucle
                }

                // on envoie le message au client
                send(fdSocketCommunication, tampon, strlen(tampon), 0);
            }
            exit(EXIT_SUCCESS);
        }
        nbClients++;
    }
    
    close(fdSocketCommunication);
    close(fdSocketAttente);

    for (int i = 0; i < MAX_CLIENTS; i++) {
        wait(NULL);
    }

    printf("Fin du programme.\n");
    return EXIT_SUCCESS;

    //fin connexion
}



int ouvrirUneSocketAttente() {
    int socketTemp;
    int longueurAdresse;
    struct sockaddr_in coordonneesServeur;

    socketTemp = socket(PF_INET, SOCK_STREAM, 0);

    if (socketTemp < 0) {
        printf("socket incorrecte\n");
        exit(EXIT_FAILURE);
    }

    // On prépare l’adresse d’attachement locale
    longueurAdresse = sizeof(struct sockaddr_in);
    memset(&coordonneesServeur, 0x00, longueurAdresse);

    // connexion de type TCP
    coordonneesServeur.sin_family = PF_INET;
    // toutes les interfaces locales disponibles
    coordonneesServeur.sin_addr.s_addr = htonl(INADDR_ANY);
    // le port d'écoute
    coordonneesServeur.sin_port = htons(PORT);

    if (bind(socketTemp, (struct sockaddr *) &coordonneesServeur, sizeof(coordonneesServeur)) == -1) {
        printf("erreur de bind\n");
        exit(EXIT_FAILURE);
    }

    if (listen(socketTemp, 5) == -1) {
        printf("erreur de listen\n");
        exit(EXIT_FAILURE);
    }

    printf("En attente de connexion...\n");

    return socketTemp;
}

int testQuitter(char tampon[]) {
    return strcmp(tampon, EXIT) == 0;
}


void motChoix(int ligneNb, FILE *fp, char mot[]){
	int count=0, i=0;
	char resultat[512];

	if(fp == NULL)
		printf("Erreur dans l'ouverture du fichier");
	else{
        // recherche un mot aléatoire parmi la liste donné
		while(fgets(resultat, sizeof(resultat), fp) != NULL){
			if(count == (ligneNb - 1)){
				sscanf(resultat, "%s", mot);
				break;
			}
			else{
				count++;
			}
		}
	}
}

char* remplirBlanc(char lettre, char* mot, char motCache[]){
    int tailleMot=strlen(mot);

	for(int i=0; i<=(tailleMot-1); i++){
		if(mot[i]==lettre){ // si le mot correspond à la lettre donné, on change le tiret par la bonne lettre
			motCache[2*i]=lettre;
		}
	}

	return motCache;
}

int checkRecurrence(char lettre, char* lettreUtilise) {
    int nb = 0;
    int taille = strlen(lettreUtilise);

    for (int i = 0; i <= (taille-1); i++)
    {
        if (lettreUtilise[i] == lettre)
        {
            nb++;
        } 
    }
    return nb;
}