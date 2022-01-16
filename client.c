#include "serveur.h"

int main(int argc , char const *argv[]) {
    int fdSocket, motRecu, longueurAdresse, tailleMot, erreur = 0, trouve = 1, resultat = 0, point = 0;
    struct sockaddr_in coordonneesServeur;
    char tampon[MAX_BUFFER]; 
    char motCache[50] = {"\0"}, mot[25], lettre, firstLettre, i;
    bool restart = true;

    fdSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (fdSocket < 0) {
        printf("socket incorrecte\n");
        exit(EXIT_FAILURE);
    }

    // On prépare les coordonnées du serveur
    longueurAdresse = sizeof(struct sockaddr_in);
    memset(&coordonneesServeur, 0x00, longueurAdresse);

    // connexion de type TCP
    coordonneesServeur.sin_family = PF_INET;
    // adresse du serveur
    inet_aton("127.0.0.1", &coordonneesServeur.sin_addr);
    // le port d'écoute du serveur
    coordonneesServeur.sin_port = htons(PORT);

    if (connect(fdSocket, (struct sockaddr *) &coordonneesServeur, sizeof(coordonneesServeur)) == -1) {
        printf("connexion impossible\n");
        exit(EXIT_FAILURE);
    }

    printf("connexion ok\n");

    while (restart == true) {
        // on attend la réponse du serveur
        motRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);

        if (motRecu > 0) {
            tampon[motRecu] = 0;
            strcpy(mot, tampon);
            tailleMot = strlen(mot);
                // remplace les lettres par des tiret
                for (int i = 0; i <= 2*(tailleMot-1); i++) {
                    if (i%2 == 0)
                    {
                        motCache[i] = '_';
                    } else{
                        motCache[i] = ' ';
                    }
                }

            //affiche la première lettre du mot    
            firstLettre = mot[0];
            trouve = checkRecurrence(firstLettre, mot);
            remplirBlanc(firstLettre, mot, motCache);
                //printf("%s ", motCache);  //Affiche un tiret pour chaque lettre
                //printf("\n%s", mot);  //Affiche le mot à trouver
        }
        
        while (erreur < 7 && trouve < tailleMot) 
        {
            bool verifLettre = false;
            printf("\nLe mot a trouver est le suivant : %s", motCache);
            printf(" en %d lettres.\n", tailleMot);

            // verification que la lettre n'a pas déjà été donné
            while (verifLettre != true){
                //demande une lettre
                entrerLettre(tampon);
                lettre = tampon[0];
                //envoie lettre
                tampon[0] = lettre;
                usleep(100000);
                send(fdSocket, tampon, strlen(tampon), 0);

                // reçoit la verification
                motRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
                if (motRecu > 0) {
                    if (tampon[0] == -1)
                    {
                        memset(tampon, 0, sizeof(tampon));
                        printf("\nAttention ! votre lettre : \"%c\" a deja ete utilise!\n", lettre);
                        verifLettre = false;
                    } else {
                        memset(tampon, 0, sizeof(tampon));
                        verifLettre = true;
                    }
                }
            }

            // attends verification lettre = mot
            motRecu = recv(fdSocket, tampon, MAX_BUFFER, 0);
            if (motRecu > 0)
            {
                resultat = tampon[0];
                memset(tampon, 0, sizeof(tampon));

                if (resultat == -1)
                {
                    erreur++;
                    affPendu(erreur);
                    printf("\nCette lettre n'est pas dans ce mot");
                } else {
                    trouve = trouve + resultat;
                    printf("\nBravo ! vous avez trouve une lettre");
                    
                    // convertit les tirets par les lettres qui ont été trouvées
                    remplirBlanc(lettre, mot, motCache);
                }
            }
        }
    

        if (erreur == 7)
        {
            affPendu(8);
            printf("\nLe mot était : %s ! Vous aurez plus de chance la prochaine fois.", mot);

            tampon[100] = true;
            usleep( 100000 );
            send(fdSocket, tampon, strlen(tampon), 0);
        } else if (trouve == tailleMot)
        {
            affPendu(9);
            printf("\nBravo ! vous avez trouve le bon mot !");
            printf("\nLe mot etait : %s !", motCache);

            point++;
            tampon[100] = true;
            usleep( 100000 );
            send(fdSocket, tampon, strlen(tampon), 0);
        }

        printf("\n\nVoulez vous recommencer ? \n");
        printf("1. Oui\n");
        printf("2. Non\n");
        printf("Votre reponse: ");
        entrerLettre(tampon);
        if (tampon[0] == '1') {
            memset(tampon, 0, sizeof(tampon));
            tampon[0] = 1;
            usleep( 100000 );
            send(fdSocket, tampon, strlen(tampon), 0);
            memset(tampon, 0, sizeof(tampon));

            /* reset variable */
            memset(motCache, 0, sizeof(motCache));
            memset(mot, 0, sizeof(mot));
            erreur = 0;
            trouve = 1;

            printf("\nPartie recommence !\n");
            restart = true;
            sleep(2);
            continue;
        }
        else {
            memset(tampon, 0, sizeof(tampon));
            tampon[0] = -1;
            usleep( 100000 );
            send(fdSocket, tampon, strlen(tampon), 0);
            memset(tampon, 0, sizeof(tampon));

            printf("\n\nPartie fini !\n");
            printf("Vous avez : %d point(s)", point);
            restart = false;
            close(fdSocket);
            return EXIT_SUCCESS;
        }   
    }
    close(fdSocket);

    return EXIT_SUCCESS;
}





/* ----- Fonction ----- */
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

char* remplirBlanc(char lettre, char* mot, char motCache[]){
    int tailleMot=strlen(mot);

	for(int i=0; i<=(tailleMot-1); i++){
		if(mot[i]==lettre){ // si le mot correspond à la lettre donné, on change le tiret par la bonne lettre
			motCache[2*i]=lettre;
		}
	}

	return motCache;
}

void entrerLettre(char tampon[]) {
    printf("Saisir une lettre :\n");
    fgets(tampon, MAX_BUFFER, stdin);
    strtok(tampon, "\n");
}