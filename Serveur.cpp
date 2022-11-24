#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/wait.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include "protocole.h" // contient la cle et la structure d'un message

int idQ,idShm,idSem, filsPub;
int fdPipe[2];
TAB_CONNEXIONS *tab;

typedef struct
{
  char  nom[20];
  char motDePasse[20];
} CLIENT;

void afficheTab();
void HandlerSIGINT(int Sig);
void reponseLogin(int expediteur, int data1, char data4[100]);


int main()
{
  int i, fd, posx = 1;
  char msg[100];
  CLIENT sclient;

  // Armement des signaux
  // Armement du signal SIGINT
  struct sigaction A;
  // Armement de SIGCHLD
  A.sa_handler = HandlerSIGINT;
  sigemptyset(&A.sa_mask);
  A.sa_flags = 0;

  if (sigaction(SIGINT,&A,NULL) == -1)
  {
    perror("Erreur de sigaction");
    exit(1);
  }
  // TO DO

  // Creation des ressources
  // Creation de la file de message
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0600)) == -1)  // CLE definie dans protocole.h
  {
    perror("(SERVEUR) Erreur de msgget");
    exit(1);
  }

  // TO BE CONTINUED

  // Creation du pipe
  // TO DO

  // Initialisation du tableau de connexions
  tab = (TAB_CONNEXIONS*) malloc(sizeof(TAB_CONNEXIONS)); 

  for (int i=0 ; i<6 ; i++)
  {
    tab->connexions[i].pidFenetre = 0;
    strcpy(tab->connexions[i].nom,"");
    tab->connexions[i].pidCaddie = 0;
  }
  tab->pidServeur = getpid();
  tab->pidPublicite = 0;

  afficheTab();

  // Creation du processus Publicite (étape 2)
  if ((idShm = shmget(CLE,52,IPC_CREAT | 0777)) == -1)
  {
    perror("Erreur de shmget");
    exit(1);
  }

  filsPub = fork();
  if (filsPub == 0)
  {
    execl("./Publicite", "Publicite", NULL);
  }

  // TO DO

  // Creation du processus AccesBD (étape 4)
  // TO DO

  MESSAGE m;
  MESSAGE reponse;

  fd = open("clients.dat",O_CREAT|O_EXCL);
  if (fd != -1)
  {
    close(fd);
  }

  while(1)
  {
  	fprintf(stderr,"(SERVEUR %d) Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      perror("(SERVEUR) Erreur de msgrcv");
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }
    switch(m.requete)
    {
      case CONNECT :  fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);
                      for (i=0;i<6;i++)
                      {
                        if (tab->connexions[i].pidFenetre == 0)
                        {
                          tab->connexions[i].pidFenetre = m.expediteur;
                          break;
                        }
                      }
                      if (i == 6)
                      {
                        fprintf(stderr, "Tout les slots sont occupées\n");
                      }
                      
                      break;

      case DECONNECT :fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);
                      for(i=0;i<6;i++)
                      {
                        if(tab->connexions[i].pidFenetre == m.expediteur)
                        {
                          tab->connexions[i].pidFenetre = 0;
                          strcpy(tab->connexions[i].nom, "");
                          break;
                        }
                      }
                      
                      break;
      case LOGIN :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%d--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.data3);
                      
                      if (m.data1 == 1)
                      {
                        strcpy(sclient.nom,m.data2);
                        strcpy(sclient.motDePasse,m.data3);

                        if((fd = open("clients.dat",O_WRONLY|O_APPEND)) == -1)
                        {
                          fprintf(stderr, "Probleme d'ouverture de fichier!\n");
                          strcpy(msg,"Probleme d'ouverture de fichier!");
                          reponseLogin(m.expediteur, 0, msg);
                        }
                        else
                        {
                          write(fd, &sclient, sizeof(CLIENT));
                          close(fd);
                          strcpy(msg, "Vous avez ete bien enregistez !");
                          reponseLogin(m.expediteur, 1, msg);
                        }
                      }
                      else
                      {
                        if((fd = open("clients.dat",O_RDONLY)) == -1)
                        {
                          fprintf(stderr, "Probleme d'ouverture du fichier\n");
                        }
                        else
                        {
                          while(posx != 0)
                          {
                            if (strcmp(sclient.nom, m.data2) == 0)
                            {
                              break;
                            }
                            posx = read(fd,&sclient, sizeof(CLIENT));
                          }
                          if (strcmp(sclient.nom, m.data2) == 0)
                          {
                            if (strcmp(sclient.motDePasse, m.data3) == 0)
                            {
                              for(i=0;i<6;i++)
                              {
                                if(tab->connexions[i].pidFenetre == m.expediteur)
                                {
                                  strcpy(tab->connexions[i].nom, m.data2);
                                  break;
                                }
                                
                              }
                              strcpy(msg, "Vous êtes connecté");
                              reponseLogin(m.expediteur, 1, msg);
                            }
                            else
                            {
                              fprintf(stderr, "Mot de passe incorrect\n");
                              strcpy(msg, "Mot de passe incorrect");
                              reponseLogin(m.expediteur, 0, msg);
                            }
                          }
                          else
                          {
                            fprintf(stderr, "Nom incorrect\n");
                            strcpy(msg,"Nom incorrect");
                            reponseLogin(m.expediteur, 0, msg);
                          }
                          close(fd);
                          
                        }
                      }    
                    
                      break; 

      case LOGOUT :   // TO DO
                      for(i=0;i<6;i++)
                      {
                        if(tab->connexions[i].pidFenetre == m.expediteur)
                        {
                          strcpy(tab->connexions[i].nom, "");
                          break;
                        }
                        
                      }
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case UPDATE_PUB :  // TO DO
                      for(i=0;i<6;i++)
                      {
                        if(tab->connexions[i].pidFenetre != 0)
                        {
                          kill(tab->connexions[i].pidFenetre, SIGUSR2);
                        }
                      }
                      break;

      case CONSULT :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case ACHAT :    // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CADDIE :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL_ALL : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case PAYER : // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                      break;

      case NEW_PUB :  // TO DO
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      break;
    }
    afficheTab();
  }
}

void afficheTab()
{
  fprintf(stderr,"Pid Serveur   : %d\n",tab->pidServeur);
  fprintf(stderr,"Pid Publicite : %d\n",tab->pidPublicite);
  fprintf(stderr,"Pid AccesBD   : %d\n",tab->pidAccesBD);
  for (int i=0 ; i<6 ; i++)
    fprintf(stderr,"%6d -%20s- %6d\n",tab->connexions[i].pidFenetre,
                                                      tab->connexions[i].nom,
                                                      tab->connexions[i].pidCaddie);
  fprintf(stderr,"\n");
}

void reponseLogin(int expediteur, int data1, char data4[100])
{
  MESSAGE reponse;
  reponse.expediteur = getpid();
  reponse.requete = LOGIN;
  reponse.type = expediteur;
  reponse.data1 = data1;
  strcpy(reponse.data4, data4);
  if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
  {
    perror("Erreur de msgnd\n");
   }
   else
  {
    printf("le msg est bien envoye\n");
  }
  kill(expediteur, SIGUSR1);
}



void HandlerSIGINT(int Sig)
{
  if (msgctl(idQ,IPC_RMID,NULL) == -1)
  {
    perror("Erreur de msgctl(2)");
  }
  if (shmctl(idShm,IPC_RMID,NULL) == -1)
  {
    perror("Erreur de shmctl");
  }
  kill(filsPub, 9);
  wait(&filsPub);
  exit(1);
}