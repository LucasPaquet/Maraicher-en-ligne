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

int i;
MESSAGE reponse;

struct sigaction A;

void handlerSIGALRM(int sig);

int main(int argc,char* argv[])
{
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);
  

  // Armement des signaux
  A.sa_handler = handlerSIGALRM;
  sigemptyset(&A.sa_mask);
  A.sa_flags = 0;
  if (sigaction(SIGALRM,&A,NULL) == -1)
  {
    perror("Erreur de sigaction");
    exit(1);
  }

  // TO DO

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(CADDIE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(CADDIE) Erreur de msgget");
    exit(1);
  }

  MESSAGE m;

  


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
                      // requete recu par le serveur, lorsque le client se log, qui transmet le pid du client au quel le caddie est relié
                      pidClient = m.expediteur;
                      fprintf(stderr,"(CADDIE %d) Requete LOGIN reçue de %d\n",getpid(),m.expediteur);
                      break;

      case LOGOUT :   // TO DO
                      // recoit la requete pour fermer le pipe et se termine
                      fprintf(stderr,"(CADDIE %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      close(fdWpipe);
                      exit(0);
                      break;

      case CONSULT :  // TO DO
                      alarm(0);
                      
                      m.expediteur = getpid();
                      // on envoie a accesDB avec le pipe la requete du client
                      write(fdWpipe, &m, sizeof(MESSAGE));

                      // on attend la reponse de AccessDB pour la renvoyer vers client
                      if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
                      {
                        perror("(CADDIE) Erreur de msgrcv after");
                        exit(1);
                      }

                      // si le produit a ete trouve, il renvoie la requete au client
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
                          kill(pidClient, SIGUSR1);
                        }
                      }
                      else
                      {
                        printf("(CADDIE)Fin de la liste\n");
                      }
                      
                    
                      fprintf(stderr,"(CADDIE %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);

                      alarm(60);
                      break;

      case ACHAT :    // TO DO
                      alarm(0);
                      fprintf(stderr,"(CADDIE %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);

                      // on transfert la requete à AccesBD
                      pidClient = m.expediteur;
                      m.expediteur = getpid();

                      write(fdWpipe, &m, sizeof(MESSAGE));

                      // on attend la réponse venant de AccesBD

                      if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
                      {
                        perror("(CADDIE) Erreur de msgrcv after");
                        exit(1);
                      }

                      // Envoi de la reponse au client
                      
                      m.expediteur = getpid();
                      m.type = pidClient;
                      if(msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long),0) == -1)
                      {
                        perror("Erreur de msgnd\n");
                      }
                      else
                      {
                        printf("(CADDIE) le result ACHAT est bien envoye\n");
                        kill(pidClient, SIGUSR1);
                      }

                      // mise a jour du caddie
                      if (atoi(m.data3) != 0)
                      {
                        articles[nbArticles].id = m.data1;
                        strcpy(articles[nbArticles].intitule, m.data2);
                        articles[nbArticles].prix = m.data5;
                        articles[nbArticles].stock = atoi(m.data3);
                        strcpy(articles[nbArticles].image, m.data4);
                        nbArticles++;
                      }
                      
                      alarm(60);
                      break;

      case CADDIE :   // TO DO
                      alarm(0);
                      fprintf(stderr,"(CADDIE %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      for (i = 0; i < nbArticles; i++) // boucle pour envoyer tout les articles dans le caddie au client
                      {
                        //on remplie la requete reponse
                        reponse.data1 = articles[i].id;
                        strcpy(reponse.data2,articles[i].intitule);
                        reponse.data5 = articles[i].prix;
                        sprintf(reponse.data3, "%d", articles[i].stock);
                        strcpy(reponse.data4,articles[i].image);

                        reponse.expediteur = getpid();
                        reponse.type = m.expediteur;
                        reponse.requete = CADDIE;
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          printf("(CADDIE) le result CADDIE est bien envoye\n");
                          kill(m.expediteur, SIGUSR1);
                        }
                      }
                      alarm(60);
                      break;

      case CANCEL :   // TO DO
                      
                      // on transmet la requete à AccesBD
                      alarm(0);
                      reponse.expediteur = getpid();
                      reponse.requete = CANCEL;

                      sprintf(reponse.data2,"%d", articles[m.data1].stock);
                      reponse.data1 = articles[m.data1].id;
                      
                      
                      write(fdWpipe, &reponse, sizeof(MESSAGE));

                      // Suppression de l'aricle du panier
                      for(i = m.data1; i< nbArticles;i++)
                      {
                        articles[i].id = articles[i+1].id;
                        strcpy(articles[i].intitule,articles[i+1].intitule);
                        articles[i].prix = articles[i+1].prix;
                        articles[i].stock = articles[i+1].stock;
                        strcpy(articles[i].image,articles[i+1].image);
                      }
                      nbArticles--;
                     
                      alarm(60);
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(CADDIE %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      alarm(0);
                      // On envoie  a AccesBD autant de requeres CANCEL qu'il y a d'articles dans le panier

                      for(i=0;i<nbArticles;i++)
                      {
                        reponse.expediteur = getpid();
                        reponse.requete = CANCEL;

                        sprintf(reponse.data2,"%d", articles[i].stock);
                        reponse.data1 = articles[i].id;
                        
                        
                        write(fdWpipe, &reponse, sizeof(MESSAGE));
                      }
                      nbArticles = 0;

                      // On vide le panier
                      alarm(60);
                      break;

      case PAYER :    // TO DO
                      alarm(0);
                      nbArticles = 0;
                      fprintf(stderr,"(CADDIE %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                      alarm(60);
                      // On vide le panier
                      break;
    }
  }
}

void handlerSIGALRM(int sig)
{
  fprintf(stderr,"(CADDIE %d) Time Out !!!\n",getpid());

  

  // Annulation du caddie et mise à jour de la BD
  for(i=0;i<nbArticles;i++)
  {
    reponse.expediteur = getpid();
    reponse.requete = CANCEL;

    sprintf(reponse.data2,"%d", articles[i].stock);
    reponse.data1 = articles[i].id;
    
    
    write(fdWpipe, &reponse, sizeof(MESSAGE));
  }
  nbArticles = 0;
  // On envoie a AccesBD autant de requetes CANCEL qu'il y a d'articles dans le panier

  // Envoi d'un Time Out au client (s'il existe toujours)
  reponse.expediteur = getpid();
  reponse.type = pidClient;
  reponse.requete = TIME_OUT;
  if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
  {
    perror("Erreur de msgnd\n");
  }
  else
  {
    kill(pidClient, SIGUSR1); 

  }
  
  close(fdWpipe); // fermer le pipe
  exit(0);

}