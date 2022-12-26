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

int main(int argc,char* argv[])
{
  char requete[200];
  char newUser[20];
  char requeteSql[200];
  MYSQL* connexion;
  MYSQL_RES  *resultat;
  MYSQL_ROW  tuple;

  MESSAGE reponse;

  int ret;
  char buffer[100];
  // Masquage de SIGINT
  sigset_t mask;
  sigaddset(&mask,SIGINT);
  sigprocmask(SIG_SETMASK,&mask,NULL);

  // Recuperation de l'identifiant de la file de messages
  fprintf(stderr,"(ACCESBD %d) Recuperation de l'id de la file de messages\n",getpid());
  if ((idQ = msgget(CLE,0)) == -1)
  {
    perror("(ACCESBD) Erreur de msgget");
    exit(1);
  }

  // Récupération descripteur lecture du pipe
  int fdRpipe = atoi(argv[1]);

  // Connexion à la base de donnée
  // TO DO
  connexion = mysql_init(NULL);
  if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
  {
    fprintf(stderr,"(ACCESBD) Erreur de connexion à la base de données...\n");
    exit(1);  
  }
  else
  {
    fprintf(stderr,"(ACCESBD) Connexion a sql reussi...\n");
  }

  MESSAGE m;

  while(1)
  {
    // Lecture d'une requete sur le pipe
    // TO DO
    if ((ret = read(fdRpipe,&m,sizeof(MESSAGE))) < 0)
    {
      perror("Erreur de read(1)"); 
      exit(1); 
    }

    switch(m.requete)
    {
      case CONSULT :  // TO DO
                      // consulte la base de données sql et renvoie la requete a caddie
                      fprintf(stderr,"(ACCESBD %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD
                      sprintf(requete,"select * from UNIX_FINAL where id = %d",m.data1); // pour recuperer les infos sur le produit en fonciton de son id
                      
                      mysql_query(connexion,requete); // execution de la requete
                      resultat = mysql_store_result(connexion);
                      if (resultat && m.data1 > 0 && m.data1 < 22)
                      {
                        tuple = mysql_fetch_row(resultat); 
                        printf("(ACCESBD) RESULTAT : %s, %s, %s, %s, %s\n", tuple[0], tuple[1], tuple[2], tuple[3], tuple[4]);
                        reponse.expediteur = getpid();
                        reponse.requete = CONSULT;
                        reponse.type = m.expediteur;
                        reponse.data1 = atoi(tuple[0]);
                        strcpy(reponse.data2, tuple[1]);
                        strcpy(reponse.data3, tuple[3]);
                        strcpy(reponse.data4, tuple[4]);
                        reponse.data5 = atof(tuple[2]);

                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          printf("(ACCESBD)Le resultat CONSULT a ete envoye\n");
                        }
                      
                      }
                      else
                      {
                        reponse.expediteur = getpid();
                        reponse.requete = CONSULT;
                        reponse.type = m.expediteur;
                        reponse.data1 = -1;
                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          printf("(ACCESBD)Le resultat CONSULT a ete envoye\n");
                        }
                      }
                      // Preparation de la reponse

                      // Envoi de la reponse au bon caddie
                      break;

      case ACHAT :    // TO DO
                      // Acces BD
                      // consulte la base de données sql et renvoie la requete a caddie
                      sprintf(requete,"select * from UNIX_FINAL where id = %d",m.data1);
                      
                      
                      mysql_query(connexion,requete);
                      resultat = mysql_store_result(connexion);
                      
                      if (resultat && m.data1 > 0 && m.data1 < 22)
                      {
                        tuple = mysql_fetch_row(resultat);
                        
                        if (atoi(tuple[3]) - atoi(m.data2) < 0)
                        {
                          // si pas assez de stock
                          strcpy(reponse.data3, "0");
                        }
                        else
                        {
                          // si assez de stock
                          sprintf(requeteSql, "update UNIX_FINAL SET stock = stock - %d where id = %d",atoi(m.data2),m.data1); // mise a jour du stock
                          mysql_query(connexion,requeteSql);
                          strcpy(reponse.data3, m.data2);
                        }

                        // Finalisation et envoi de la reponse
                        reponse.expediteur = getpid();
                        reponse.requete = ACHAT;
                        reponse.type = m.expediteur;
                        reponse.data1 = atoi(tuple[0]);
                        strcpy(reponse.data2, tuple[1]);
                        strcpy(reponse.data4, tuple[4]);
                        reponse.data5 = atof(tuple[2]);

                        if(msgsnd(idQ, &reponse, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                          printf("(ACCESBD)Le resultat AHCAT a ete envoye\n");
                        }
                      
                      }

                      
                      break;

      case CANCEL :   // TO DO
                      // Acces BD

                      // Mise à jour du stock en BD
                      sprintf(requeteSql, "update UNIX_FINAL SET stock = stock + %d where id = %d",atoi(m.data2),m.data1);
                      mysql_query(connexion,requeteSql);
                      break;

    }
  }
}
