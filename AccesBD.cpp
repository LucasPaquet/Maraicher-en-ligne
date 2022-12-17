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
                      fprintf(stderr,"(ACCESBD %d) Requete CONSULT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD
                      sprintf(requete,"select * from UNIX_FINAL where id = %d",m.data1);
                      
                      mysql_query(connexion,requete);
                      resultat = mysql_store_result(connexion);
                      if (resultat && m.data1 > 0 && m.data1 < 22)
                      {
                        tuple = mysql_fetch_row(resultat); 
                        // fprintf(stderr,"RESULTAT : %s \n", tuple[0]);
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
                      fprintf(stderr,"(ACCESBD %d) Requete ACHAT reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      // Finalisation et envoi de la reponse
                      break;

      case CANCEL :   // TO DO
                      fprintf(stderr,"(ACCESBD %d) Requete CANCEL reçue de %d\n",getpid(),m.expediteur);
                      // Acces BD

                      // Mise à jour du stock en BD
                      break;

    }
  }
}
