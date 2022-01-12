#include "serveur.h"

int main(int argc, char const *argv[]) {
    FILE* fp = NULL;
    int fdSocketAttente, fdSocketCommunication, nbRecu, tailleMot, ligneNb, erreur = 0, trouve = 1, resultat = 0;
    struct sockaddr_in coordonneesAppelant;
    struct sockaddr_in coordonneesServeur;
    char tampon[MAX_BUFFER], motCache[50] = {"\0"};
    char mot[25], lettreUtiliseFaux[7] = { '\0' }, lettreUtiliseCorrect[25] = { '\0' }, lettre, firstLettre;

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
                remplirBlanc(firstLettre, mot, motCache);
                lettreUtiliseCorrect[0] = firstLettre;
                    //printf("%s ", motCache);  //Affiche un tiret pour chaque lettre
                    //printf("\n%s", mot);  //Affiche le mot à trouver

                // on attend le message du client
                // la fonction recv est bloquante
                nbRecu = recv(fdSocketCommunication, tampon, MAX_BUFFER, 0);

                if (nbRecu > 0) {
                    tampon[nbRecu] = 0;
                    printf("Recu de %s:%d : %s\n",
                           inet_ntoa(coordonneesAppelant.sin_addr),
                           ntohs(coordonneesAppelant.sin_port),
                           tampon);

                    if (testQuitter(tampon)) {
                        break; // on quitte la boucle
                    }
                }

                lireMessage(tampon);

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
    remplirBlanc(firstLettre, mot, motCache);
    lettreUtiliseCorrect[0] = firstLettre;
        //printf("%s ", motCache);  //Affiche un tiret pour chaque lettre
        //printf("\n%s", mot);  //Affiche le mot à trouver  

    while (erreur < 7 && trouve < tailleMot)
    {
        bool verifLettre = false;
        printf("\nLe mot a trouver est le suivant : %s", motCache);
        printf(" en %d lettres.\n", tailleMot);

        // verification que la lettre n'a pas déjà été donné
        while (verifLettre != true){
            printf("\nentrer une lettre : ");
            scanf(" %c", &lettre);
            //verification

            int nbFaux = checkRecurrence(lettre, lettreUtiliseFaux);
            int nbCorrect = checkRecurrence(lettre, lettreUtiliseCorrect);

            if (nbFaux > 0 || nbCorrect > 0)
            {
                printf("\nAttention ! votre lettre : \"%c\" a deja ete utilise!\n", lettre);
                verifLettre = false;
            } else {
                verifLettre = true;
            }
        }
        
        //verification que la lettre donné est bien dans le mot
        resultat = checkRecurrence(lettre, mot);
        
        {
            static int faux = 0, correct = 0;
            if (resultat == 0)
            {
                lettreUtiliseFaux[faux] = lettre;
                faux++;
                erreur++;
                affPendu(erreur);
                printf("\nCette lettre n'est pas dans ce mot");
            } else {
                lettreUtiliseCorrect[correct] = lettre;
                correct++;
                trouve= trouve + resultat;
                printf("\nBravo ! vous avez trouve une lettre");
            }

        }
        // convertit les tirets par les lettres qui ont été trouvées
        remplirBlanc(lettre, mot, motCache);
    }
    

    if (erreur == 7)
    {
        affPendu(8);
        printf("\nLe mot était : %s ! Vous aurez plus de chance la prochaine fois.", mot);
    } else if (trouve == tailleMot)
    {
        affPendu(9);
        printf("\nBravo ! vous avez trouve le bon mot !");
        printf("\nLe mot etait : %s !", motCache);
    }
    



    return EXIT_SUCCESS;
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

void lireMessage(char tampon[]) {
    printf("Saisir un message à envoyer :\n");
    fgets(tampon, MAX_BUFFER, stdin);
    strtok(tampon, "\n");
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