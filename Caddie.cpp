#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <signal.h>
#include <mysql.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ;

ARTICLE articles[10];
int nbArticles = 0;

int fdWpipe;
int pidClient;



void handlerSIGALRM(int sig);

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Armement des signaux
  // TO DO

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }

  MESSAGE m;
  MESSAGE reponse;
  


  // Récupération descripteur écriture du pipe
  fdWpipe = atoi(argv[1]);
  
  while(1)
  {
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
    {
      perror("(CADDIE) Erreur de msgrcv boucle");
      exit(1);
    }

    switch(m.requete)
    {
      case LOGIN :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);
                      break;

      case LOGOUT :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      
                      exit(0);
                      break;

      case CONSULT :  // TO DO
                      
                      pidClient = m.expediteur;
                      printf("(CADDIE)Le client est %d\n", pidClient);
                      m.expediteur = getpid();
                      write(fdWpipe, &m, sizeof(MESSAGE));

                      if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
                      {
                        perror("(CADDIE) Erreur de msgrcv after");
                        exit(1);
                      }
                      if (m.data1 != -1)
                      {
                        m.expediteur = getpid();
                        m.type = pidClient;
                        if(msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          printf("(CADDIE) le result CONSULT est bien envoye\n");
                          int debug = kill(pidClient, SIGUSR1);
                          printf("DEBUG : %d et %d\n", debug, pidClient);
                        }
                      }
                      else
                      {
                        printf("(CADDIE)Fin de la liste\n");
                      }
                      
                    
                      fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);

                     
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      // on transfert la requete à AccesBD
                      
                      // on attend la réponse venant de AccesBD
                        
                      // Envoi de la reponse au client

                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);

                      // on transmet la requete à AccesBD

                      // Suppression de l'aricle du panier
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);

                      // On envoie a AccesBD autant de requeres CANCEL qu'il y a d'articles dans le panier

                      // On vide le panier
                      break;

      case PAYER :    // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);

                      // On vide le panier
                      break;
    }
  }
}

void handlerSIGALRM(int sig)
{
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());

  // Annulation du caddie et mise à jour de la BD
  // On envoie a AccesBD autant de requetes CANCEL qu'il y a d'articles dans le panier

  // Envoi d'un Time Out au client (s'il existe toujours)
         
  exit(0);
}