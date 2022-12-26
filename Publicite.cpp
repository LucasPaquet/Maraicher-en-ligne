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
#include "protocole.h" // contient la cle et la structure d'un message

int idQ, idShm;
char *pShm;
char temp;
char pub[51];
void handlerSIGUSR1(int sig);
int fd;
MESSAGE requete;

int main()
{
  // Armement des signaux
  // TO DO

  // Masquage des signaux
  sigset_t mask;
  sigfillset(&mask);
  sigdelset(&mask,SIGUSR1);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  struct sigaction A;
  A.sa_handler = handlerSIGUSR1;
  sigemptyset(&A.sa_mask);
  A.sa_flags = 0;

  if(sigaction(SIGUSR1,&A,NULL) == -1)
  {
      perror("Erreur de sigaction");
      exit(1);
  }

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(PUBLICITE %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(PUBLICITE) Erreur de msgget");
    exit(1);
  }

  // Recuperation de l'identifiant de la mémoire partagée
  if ((idShm = shmget(CLE,0,0)) == -1)
  {
    perror("Erreur de shmget");
    exit(1);
  }

  // Attachement à la mémoire partagée
  
  if ((pShm = (char*)shmat(idShm,NULL,0)) == (char*)-1)
  {
    perror("Erreur de shmat");
    exit(1);
  }
  // Mise en place de la publicité en mémoire partagée
  
  strcpy(pub,"Comment allez vous ?");

  for (int i=0 ; i<=50 ; i++) pShm[i] = ' ';
  pShm[51] = '\0';
  for (int i=0 ; i<strlen(pub) ; i++) pShm[i] = pub[i];

  while(1)
  {
    // Envoi d'une requete LOGIN au serveur
    requete.expediteur = getpid();
    requete.requete = UPDATE_PUB;
    requete.type = 1;
    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
    }

    sleep(1); 

    // Decallage vers la gauche
    temp = pShm[0];
    
    for (int i=0 ;i<50;i++)
    {
      
      pShm[i] = pShm[i+1];

    }
    pShm[50] = temp;
    pShm[51] = '\0';
  }
}

void handlerSIGUSR1(int sig)
{
  fprintf(stderr,"(PUBLICITE %d) Nouvelle publicite !\n",getpid());
  if (msgrcv(idQ,&requete,sizeof(MESSAGE)-sizeof(long),getpid(),0) == -1)
  {
    perror("(PUBLICITE) Erreur de msgrcv");
    exit(1);
  }
  // Lecture message NEW_PUB
  strcpy(pub,requete.data4);

  for (int i=0 ; i<=50 ; i++) pShm[i] = ' ';
  pShm[51] = '\0';
  for (int i=0 ; i<strlen(pub) ; i++) pShm[i] = pub[i];

  // Mise en place de la publicité en mémoire partagée
}
