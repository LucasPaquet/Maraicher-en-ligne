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
#include <setjmp.h>

int idQ,idShm,idSem, filsPub, filsCaddie, filsAccessBD; 
int fdPipe[2];
TAB_CONNEXIONS *tab;

typedef struct
{
  char  nom[20];
  char motDePasse[20];
} CLIENT;

void afficheTab();
void HandlerSIGINT(int Sig);
void HandlerSIGCHLD(int Sig);
void reponseLogin(int expediteur, int data1, char data4[100]);

int sem_signal(int num);//la fonctions pour les semaphores


sigjmp_buf contexte; // pour le setjmp (ne pas faire crash le serveur apres un SIGHCHLD)

int retour; // pour le setjmp

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

  struct sigaction B;

  // Armement du signal SIGCHLD
  B.sa_handler = HandlerSIGCHLD;
  sigemptyset(&B.sa_mask);
  B.sa_flags = 0;

  if (sigaction(SIGCHLD,&B,NULL) == -1)
  {
    perror("Erreur de sigaction");
    exit(1);
  }

  // TO DO

  // Creation des ressources

  // Creation de semaphore
  if ((idSem = semget(CLE,1,IPC_CREAT | IPC_EXCL | 0600)) == -1)
  {
    perror("Erreur de semget");
    exit(1);
  }


  // Creation de la file de message
  fprintf(stderr,"(SERVEUR %d) Creation de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,IPC_CREAT | IPC_EXCL | 0600)) == -1)  // CLE definie dans protocole.h
  {
    perror("(SERVEUR) Erreur de msgget");
    exit(1);
  }
  else
  {
    printf("(SERVEUR) msgget reussi\n");
  }

  // TO BE CONTINUED

  // Creation du pipe
  if (pipe(fdPipe) == -1)
  {
    perror("Erreur de pipe");
    exit(1);
  }
 
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

  // Creation de la memoire partage et du processus Publicite (étape 2)  
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
  tab->pidPublicite = filsPub; // on mets a joue la table des connexions

  // TO DO

  //Creation du processus AccesBD (étape 4)

  filsAccessBD = fork();
  if (filsAccessBD == 0)
  {
    char str[10];
    sprintf(str, "%d", fdPipe[0]); // on convertit le fdPipe[0](int) en char car execl() ne prend que des char en options
    execl("./AccesBD", "AccesBD", str, NULL);
  }
  tab->pidAccesBD = filsAccessBD; // on mets a joue la table des connexions

  // TO DO

  MESSAGE m;
  MESSAGE reponse;

  retour=sigsetjmp(contexte,1); // permet de ne pas faire crash le serveur, apres que un fils soit zombie et nettoyé, on remet le serveur ici (VOIR handlerSIGCHLD)

  while(1)
  {
  	fprintf(stderr,"(SERVEUR %d) Attente d'une requete...\n",getpid());
    if (msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),1,0) == -1)
    {
      msgctl(idQ,IPC_RMID,NULL);
      exit(1);
    }
    switch(m.requete)
    {
      case CONNECT :  // Requete recu lorsque le Client vient de se lancer (la fenetre)
                      fprintf(stderr,"(SERVEUR %d) Requete CONNECT reçue de %d\n",getpid(),m.expediteur);
                      for (i=0;i<6;i++)
                      {
                        if (tab->connexions[i].pidFenetre == 0) // on recherche une place libre dans la table de connections
                        {
                          tab->connexions[i].pidFenetre = m.expediteur;
                          break;
                        }
                      }
                      if (i == 6)
                      {
                        fprintf(stderr, "Tout les slots sont occupées\n");
                      }
                      afficheTab();
                      break;

      case DECONNECT :// requette recu quand la fenetre du client se ferme
                      fprintf(stderr,"(SERVEUR %d) Requete DECONNECT reçue de %d\n",getpid(),m.expediteur);

                      for(i=0;i<6;i++)
                      {
                        if(tab->connexions[i].pidFenetre == m.expediteur) // on cherche son pid dans la taable de connexion pour pouvoir la vidé
                        {
                          tab->connexions[i].pidFenetre = 0;
                          strcpy(tab->connexions[i].nom, "");
                          break;
                        }
                      }
                      afficheTab();
                      
                      break;
      case LOGIN :    // TO DO
                      // Requete recu du client lorsqu'il essaye de se connecter (nouvel et ancien utilisateur)
                      if (sem_signal(0) != -1) // verifier que le gerant n'est pas actif
                      {
                        fprintf(stderr,"(SERVEUR %d) Requete LOGIN reçue de %d : --%d--%s--%s--\n",getpid(),m.expediteur,m.data1,m.data2,m.data3);
                      
                        if (m.data1 == 1) // si l'utilisateur a coché la case "Nouveau client"
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
                            filsCaddie = fork(); // A chaque client « loggé » va correspondre un processus Caddie
                            if (filsCaddie == 0)
                            {
                              char str[10];
                              sprintf(str, "%d", fdPipe[1]); // on convertit l'entree du pipe car execl() ne prend que des char
                              execl("./Caddie", "Caddie", str, NULL);// on passe l'entre du pipe
                            }
                            for(i=0;i<6;i++)
                            {
                              if(tab->connexions[i].pidFenetre == m.expediteur) // on cherche son pid de fenetre pour pouvoir remplir la table de connexion avec son bon caddie et login
                              {
                                strcpy(tab->connexions[i].nom, m.data2);
                                tab->connexions[i].pidCaddie = filsCaddie;

                                // on envoie une requette de login au caddie pour qu'il memorise le pid de son client
                                reponse.expediteur = m.expediteur;
                                reponse.type = filsCaddie;
                                reponse.requete = LOGIN;
                                if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                                {
                                  perror("Erreur de msgnd\n");
                                }
                                break;
                              }
                              
                            }
                            write(fd, &sclient, sizeof(CLIENT));// on ecrit les logs et mdp du client dans le ficher "clients.dat"
                            close(fd);
                            strcpy(msg, "Vous avez bien été enregistré et connecté!"); // envoie du msg de la connexion du client
                            reponseLogin(m.expediteur, 1, msg);
                          }
                        }
                        else // si l'utilisateur N'A PAS coché la case "Nouveau client"
                        {
                          if((fd = open("clients.dat",O_RDONLY)) == -1)
                          {
                            fprintf(stderr, "Probleme d'ouverture du fichier\n");
                          }
                          else
                          {
                            do
                            {
                              if (strcmp(sclient.nom, m.data2) == 0) // si le login correspond a une login du fichier
                              {
                                break; // on sort de la boucle
                              }
                              posx = read(fd,&sclient, sizeof(CLIENT));
                            }while(posx != 0); // tant que le fichier n'est pas completement parcourue

                            if (strcmp(sclient.nom, m.data2) == 0) // on verifie si le login correspond a une login du fichier
                            {
                              // le nom est bon

                              if (strcmp(sclient.motDePasse, m.data3) == 0) // Si le mdp est bon
                              {
                                filsCaddie = fork(); // A chaque client « loggé » va correspondre un processus Caddie
                                if (filsCaddie == 0)
                                {
                                  char str[10];
                                  sprintf(str, "%d", fdPipe[1]); // on convertit l'entree du pipe
                                  execl("./Caddie", "Caddie", str, NULL);// on passe l'entre du pipe
                                }
                                for(i=0;i<6;i++)
                                {
                                  if(tab->connexions[i].pidFenetre == m.expediteur) // on cherche son pid de fenetre pour pouvoir remplir la table de connexion avec son bon caddie et login
                                  {
                                    strcpy(tab->connexions[i].nom, m.data2);
                                    tab->connexions[i].pidCaddie = filsCaddie;

                                     // on envoie une requette de login au caddie pour qu'il memorise le pid de son client
                                    reponse.expediteur = m.expediteur;
                                    reponse.type = filsCaddie;
                                    reponse.requete = LOGIN;
                                    if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                                    {
                                      perror("Erreur de msgnd\n");
                                    }

                                    break;
                                  }
                                  
                                }

                                // envoie du msg de la connexion du client
                                strcpy(msg, "Vous êtes connecté");
                                reponseLogin(m.expediteur, 1, msg);
                                
                              }
                              else
                              {
                                fprintf(stderr, "Mot de passe incorrect\n");
                                strcpy(msg, "Mot de passe incorrect");
                                // envoie du msg d'erreur au client
                                reponseLogin(m.expediteur, 0, msg);
                              }
                            }
                            else
                            {
                              // envoie du msg d'erreur au clie
                              fprintf(stderr, "Nom incorrect\n");
                              strcpy(msg,"Nom incorrect");
                              reponseLogin(m.expediteur, 0, msg);
                            }
                            close(fd);
                            
                          }
                        }    
                        afficheTab();
                      }
                      else // si le gerant est connecte
                      {
                        // la requête ne peut pas être traitée. En réponse, il envoie une requête BUSY au processus client.
                        reponse.requete = BUSY;
                        reponse.expediteur = getpid();
                        reponse.type = m.expediteur;
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          kill(m.expediteur, SIGUSR1); // pour le signaler que le client a un msg
                        }
                      }
                      
                      break; 

      case LOGOUT :   // TO DO
                      // requete recu quand le client se deconnecte (la fenetre reste ouverte)
                      for(i=0;i<6;i++)
                      {
                        if(tab->connexions[i].pidFenetre == m.expediteur) // on recherche son pid pour lui enlever son caddie et son login
                        {
                          m.type = tab->connexions[i].pidCaddie;
                          reponse.type = tab->connexions[i].pidCaddie;
                          tab->connexions[i].pidCaddie = 0;
                          strcpy(tab->connexions[i].nom, "");
                          break;
                        }
                        
                      }

                      m.requete = CANCEL_ALL; // on envoye CANCEL_ALL car elle permet deja de suppr tout les articles et de les remttres dans la bases de donnés
                      
                      if(msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long),0) == -1)
                      {
                        perror("Erreur de msgnd\n");
                      }

                      
                      //envoie requette logout au caddie pour qu'il se termine
                      reponse.expediteur = getpid();
                      reponse.requete = LOGOUT;
                      if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                      {
                        perror("Erreur de msgnd\n");
                      }

                      afficheTab();
                      
                     
                      fprintf(stderr,"(SERVEUR %d) Requete LOGOUT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case UPDATE_PUB :  // TO DO
                      // appeler chaque seconde, pour envoyer un signal a chaque client que le decalage s'est fait
                      for(i=0;i<6;i++)
                      {
                        if(tab->connexions[i].pidFenetre != 0) // pour envoyer le signal a toute les fenetre (me celle qui ne sont pas logge)
                        {
                          kill(tab->connexions[i].pidFenetre, SIGUSR2);
                        }
                      }
                      break;

      case CONSULT :  // TO DO
                      // transmet la requete au caddie correspondant au client
                      if (sem_signal(0) != -1) // verifier que le gerant n'est pas actif
                      {
                        reponse.expediteur = m.expediteur;
                        reponse.requete = CONSULT;
                        for(i=0;i<6;i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur) // on recherch le caddie de l'expediteur dans la table de connection
                          {
                            reponse.type = tab->connexions[i].pidCaddie;
                            break;
                          }
                        }
                        
                        reponse.data1 = m.data1;
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                            printf("(SERVEUR)le consult est bien envoye id : %d\n", reponse.data1);
                        }
                        
                      }
                      else // si le gerant est actif
                      {
                        reponse.requete = BUSY;
                        reponse.expediteur = getpid();
                        reponse.type = m.expediteur;
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          kill(m.expediteur, SIGUSR1);
                        }
                      }
                      fprintf(stderr,"(SERVEUR %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      break;

      case ACHAT :    // TO DO
                      // transmet la requete au caddie correspondant au client
                      if (sem_signal(0) != -1) // verifier que le gerant n'est pas actif
                      {
                        
                        for(i=0;i<6;i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;
                            break;
                          }
                          
                        }
                        
                        if(msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                            printf("(SERVEUR)le consult est bien envoye id : %d\n", reponse.data1);
                        }
                      }
                      else // si le gerant est connnecte
                      {
                        reponse.requete = BUSY;
                        reponse.expediteur = getpid();
                        reponse.type = m.expediteur;
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          kill(m.expediteur, SIGUSR1);
                        }
                      }
                      fprintf(stderr,"(SERVEUR %d) Requete ACHAT reçue de %d et \n",m.expediteur);

                      break;

      case CADDIE :   // TO DO
                      // transmet la requete au caddie correspondant au client pour pouvoir mettre a jour son caddie
                      for(i=0;i<6;i++)
                      {
                        if(tab->connexions[i].pidFenetre == m.expediteur)
                        {
                          m.type = tab->connexions[i].pidCaddie;
                          break;
                        }
                        
                      }
                      
                      if(msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long),0) == -1)
                      {
                        perror("Erreur de msgnd\n");
                      }
                      
                      fprintf(stderr,"(SERVEUR %d) Requete CADDIE reçue de %d\n",getpid(),m.expediteur);
                      break;

      case CANCEL :   // TO DO
                      // transmet la requete au caddie correspondant au client pour pouvoir mettre a jour son caddie (supprimer un article)
                      if (sem_signal(0) != -1)
                      {
                        for(i=0;i<6;i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;
                            break;
                          }
                          
                        }
                        
                        if(msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        
                      }
                      else
                      {
                        reponse.requete = BUSY;
                        reponse.expediteur = getpid();
                        reponse.type = m.expediteur;
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          kill(m.expediteur, SIGUSR1);
                        }
                      }
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL reçue de %d\n",getpid(),m.data1);
                      break;

      case CANCEL_ALL : // TO DO
                      // transmet la requete au caddie correspondant au client pour pouvoir remettre a zero son caddie
                      if (sem_signal(0) != -1)
                      {
                        for(i=0;i<6;i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;
                            break;
                          }
                          
                        }
                        
                        if(msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        
                      }
                      else
                      {
                        reponse.requete = BUSY;
                        reponse.expediteur = getpid();
                        reponse.type = m.expediteur;
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          kill(m.expediteur, SIGUSR1);
                        }
                      }
                      fprintf(stderr,"(SERVEUR %d) Requete CANCEL_ALL reçue de %d\n",getpid(),m.expediteur);
                      break;

      case PAYER : // TO DO
                    // transmet la requete au caddie correspondant au client 

                      if (sem_signal(0) != -1)
                      {
                        for(i=0;i<6;i++)
                        {
                          if(tab->connexions[i].pidFenetre == m.expediteur)
                          {
                            m.type = tab->connexions[i].pidCaddie;
                            break;
                          }
                          
                        }
                        
                        if(msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                      }
                      else
                      {
                        reponse.requete = BUSY;
                        reponse.expediteur = getpid();
                        reponse.type = m.expediteur;
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          kill(m.expediteur, SIGUSR1);
                        }
                      }
                      
                      fprintf(stderr,"(SERVEUR %d) Requete PAYER reçue de %d\n",getpid(),m.expediteur);
                      break;

      case NEW_PUB :  // TO DO
                      // Le processus Gérant envoie une requête NEW_PUB au serveur via la file de
                      // messages. Cette requête contient la nouvelle publicité (champ data4)
                      // Le serveur reçoit cette requête et la transmet (via la file de messages) au processus Publicité
                      m.type = filsPub;
                      if(msgsnd(idQ, &m, sizeof(MESSAGE) - sizeof(long),0) == -1)
                      {
                        perror("Erreur de msgnd\n");
                        
                      }
                      else
                      {
                        kill(filsPub, SIGUSR1);
                      }
                      fprintf(stderr,"(SERVEUR %d) Requete NEW_PUB reçue de %d\n",getpid(),m.expediteur);
                      break;
    }
    //afficheTab();
  }
}

void afficheTab() // fonction pour afficher la table de connextion
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

void reponseLogin(int expediteur, int data1, char data4[100]) // fonction pour envoyer la reponse de la requete login au client data1 = 1 si loggé, 0 = echoué, data4 c'est le msg
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
    printf("(SERVEUR)la reponseLogin est bien envoye\n");
  }
  kill(expediteur, SIGUSR1);
}



void HandlerSIGINT(int Sig)
{
  // suprimmer la file de msg
  if (msgctl(idQ,IPC_RMID,NULL) == -1)
  {
    perror("Erreur de msgctl(2)");
  }
  // suprimmer la memoire partage
  if (shmctl(idShm,IPC_RMID,NULL) == -1)
  {
    perror("Erreur de shmctl");
  }
  // Suppression de l’ensemble de semaphores
  if (semctl(idSem,0,IPC_RMID) == -1)
  {
    perror("Erreur de semctl (3)");
  }
  kill(filsPub, 9); // kill le processus Publicite
  kill(filsAccessBD, 9); // kill le processus AccesDB
  wait(NULL); // netoyer les zombie
  close(fdPipe[0]); // fermeture du pipe pour accessDB et Caddie
  close(fdPipe[1]);
  exit(1); // terminer le serveur
}

void HandlerSIGCHLD(int Sig)
{
  // pour netoyer proprement les zombies
  printf("(SERVEUR) enfant zombie\n");
  int fils;
  fils = wait(NULL); // on recupere le pid du fils zombie pour supprimer le pid du caddie
  for(int i=0;i<6;i++)
  {
 
      if(tab->connexions[i].pidCaddie == fils)
      {
        tab->connexions[i].pidCaddie = 0;
        break;
      }
    
  }
  siglongjmp(contexte,10); // permet de ne pas faire crash le serveur, apres que un fils soit zombie et nettoyé, on remet le serveur juste avant le while
}

// Fonction pour verifier la semaphore
// Lorsqu’il reçoit une requête, il doit vérifier que le Gérant n’est pas actif. Pour
// cela, il réalise une tentative de prise non bloquante du sémaphore 

int sem_signal(int num)
{
  struct sembuf action;
  action.sem_num = num;
  action.sem_op = 0; // on ne modifie pas la valeur du semaphore (seul le gerant le fait)
  action.sem_flg = SEM_UNDO | IPC_NOWAIT; // IPC_NOWAIT car on ne veut pas que se soit bloquant
  return semop(idSem,&action,1); // return -1 si le semaphore est a 0 (gerant actif), sinon != -1 si semaphore = 1
}
