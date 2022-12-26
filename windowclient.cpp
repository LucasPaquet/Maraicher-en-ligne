#include "windowclient.h"
#include "ui_windowclient.h"
#include <QMessageBox>
#include <string>
using namespace std;

#include "protocole.h"

#include <sys/types.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <signal.h>

extern WindowClient *w;

int idQ, idShm;
bool logged;
char* pShm;
ARTICLE articleEnCours;
float totalCaddie = 0.0;

MESSAGE requete;

void handlerSIGUSR1(int sig);
void handlerSIGUSR2(int sig);

#define REPERTOIRE_IMAGES "images/"

WindowClient::WindowClient(QWidget *parent) : QMainWindow(parent), ui(new Ui::WindowClient)
{
    ui->setupUi(this);

    // Configuration de la table du panier (ne pas modifer)
    ui->tableWidgetPanier->setColumnCount(3);
    ui->tableWidgetPanier->setRowCount(0);
    QStringList labelsTablePanier;
    labelsTablePanier << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetPanier->setHorizontalHeaderLabels(labelsTablePanier);
    ui->tableWidgetPanier->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetPanier->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetPanier->horizontalHeader()->setVisible(true);
    ui->tableWidgetPanier->horizontalHeader()->setDefaultSectionSize(160);
    ui->tableWidgetPanier->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetPanier->verticalHeader()->setVisible(false);
    ui->tableWidgetPanier->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de l'identifiant de la file de messages
    fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la file de messages\n",getpid());
    // TO DO
    if ((idQ = msgget(CLE,0)) == -1)
    {
      perror("Erreur de msgget");
    }

    printf("idQ = %d\n",idQ);



    // Recuperation de l'identifiant de la mémoire partagée
    fprintf(stderr,"(CLIENT %d) Recuperation de l'id de la mémoire partagée\n",getpid());
    if ((idShm = shmget(CLE,0,0)) == -1)
    {
      perror("Erreur de shmget");
      exit(1);
    }
    // TO DO

    // Attachement à la mémoire partagée
    if ((pShm = (char*)shmat(idShm,NULL,SHM_RDONLY)) == (char*)-1)
    {
      perror("Erreur de shmat");
      exit(1);
    }
    // TO DO

    // Armement des signaux
    // TO DO
    // Armement du signal SIGUSR1
    struct sigaction A;
    A.sa_handler = handlerSIGUSR1;
    sigemptyset(&A.sa_mask);
    A.sa_flags = 0;

    if(sigaction(SIGUSR1,&A,NULL) == -1)
    {
        perror("Erreur de sigaction");
        exit(1);
    }
    // Armement du signal SIGUSR2
    A.sa_handler = handlerSIGUSR2;
    sigemptyset(&A.sa_mask);
    A.sa_flags = 0;

    if(sigaction(SIGUSR2,&A,NULL) == -1)
    {
        perror("Erreur de sigaction");
        exit(1);
    }

    // Envoi d'une requete de connexion au serveur
    // TO DO
    requete.expediteur = getpid();
    requete.requete = CONNECT;
    requete.type = 1;

    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
    }
    else
    {
      printf("la connexion est bien envoye\n");
    }

}

WindowClient::~WindowClient()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setNom(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditNom->clear();
    return;
  }
  ui->lineEditNom->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getNom()
{
  strcpy(nom,ui->lineEditNom->text().toStdString().c_str());
  return nom;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setMotDePasse(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditMotDePasse->clear();
    return;
  }
  ui->lineEditMotDePasse->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowClient::getMotDePasse()
{
  strcpy(motDePasse,ui->lineEditMotDePasse->text().toStdString().c_str());
  return motDePasse;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setPublicite(const char* Text)
{
  if (strlen(Text) == 0 )
  {
    ui->lineEditPublicite->clear();
    return;
  }
  ui->lineEditPublicite->setText(Text);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setImage(const char* image)
{
  // Met à jour l'image
  char cheminComplet[80];
  sprintf(cheminComplet,"%s%s",REPERTOIRE_IMAGES,image);
  QLabel* label = new QLabel();
  label->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
  label->setScaledContents(true);
  QPixmap *pixmap_img = new QPixmap(cheminComplet);
  label->setPixmap(*pixmap_img);
  label->resize(label->pixmap()->size());
  ui->scrollArea->setWidget(label);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::isNouveauClientChecked()
{
  if (ui->checkBoxNouveauClient->isChecked()) return 1;
  return 0;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setArticle(const char* intitule,float prix,int stock,const char* image)
{
  ui->lineEditArticle->setText(intitule);
  if (prix >= 0.0)
  {
    char Prix[20];
    sprintf(Prix,"%.2f",prix);
    ui->lineEditPrixUnitaire->setText(Prix);
  }
  else ui->lineEditPrixUnitaire->clear();
  if (stock >= 0)
  {
    char Stock[20];
    sprintf(Stock,"%d",stock);
    ui->lineEditStock->setText(Stock);
  }
  else ui->lineEditStock->clear();
  setImage(image);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getQuantite()
{
  return ui->spinBoxQuantite->value();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::setTotal(float total)
{
  if (total >= 0.0)
  {
    char Total[20];
    sprintf(Total,"%.2f",total);
    ui->lineEditTotal->setText(Total);
  }
  else ui->lineEditTotal->clear();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::loginOK()
{
  ui->pushButtonLogin->setEnabled(false);
  ui->pushButtonLogout->setEnabled(true);
  ui->lineEditNom->setReadOnly(true);
  ui->lineEditMotDePasse->setReadOnly(true);
  ui->checkBoxNouveauClient->setEnabled(false);

  ui->spinBoxQuantite->setEnabled(true);
  ui->pushButtonPrecedent->setEnabled(true);
  ui->pushButtonSuivant->setEnabled(true);
  ui->pushButtonAcheter->setEnabled(true);
  ui->pushButtonSupprimer->setEnabled(true);
  ui->pushButtonViderPanier->setEnabled(true);
  ui->pushButtonPayer->setEnabled(true);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::logoutOK()
{
  ui->pushButtonLogin->setEnabled(true);
  ui->pushButtonLogout->setEnabled(false);
  ui->lineEditNom->setReadOnly(false);
  ui->lineEditMotDePasse->setReadOnly(false);
  ui->checkBoxNouveauClient->setEnabled(true);

  ui->spinBoxQuantite->setEnabled(false);
  ui->pushButtonPrecedent->setEnabled(false);
  ui->pushButtonSuivant->setEnabled(false);
  ui->pushButtonAcheter->setEnabled(false);
  ui->pushButtonSupprimer->setEnabled(false);
  ui->pushButtonViderPanier->setEnabled(false);
  ui->pushButtonPayer->setEnabled(false);

  setNom("");
  setMotDePasse("");
  ui->checkBoxNouveauClient->setCheckState(Qt::CheckState::Unchecked);

  setArticle("",-1.0,-1,"");

  w->videTablePanier();
  totalCaddie = 0.0;
  w->setTotal(-1.0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du panier (ne pas modifier) /////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::ajouteArticleTablePanier(const char* article,float prix,int quantite)
{
    char Prix[20],Quantite[20];

    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetPanier->rowCount();
    nbLignes++;
    ui->tableWidgetPanier->setRowCount(nbLignes);
    ui->tableWidgetPanier->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetPanier->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetPanier->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetPanier->setItem(nbLignes-1,2,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::videTablePanier()
{
    ui->tableWidgetPanier->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowClient::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetPanier->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions permettant d'afficher des boites de dialogue (ne pas modifier ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueMessage(const char* titre,const char* message)
{
   QMessageBox::information(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::dialogueErreur(const char* titre,const char* message)
{
   QMessageBox::critical(this,titre,message);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::closeEvent(QCloseEvent *event)
{
  // TO DO (étape 1)µ
  // Attention aussi que si un utilisateur clique sur la croix de la fenêtre alors qu’il est loggé, une
  // requête LOGOUT doit être envoyée au serveur avant l’envoi de la requête DECONNECT.
  // Seulement après, le processus Client peut se terminer.

    // envoi d'un logout si logged
    requete.expediteur = getpid();
    requete.requete = LOGOUT;
    requete.type = 1;
    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
     }
     else
    {
      printf("le logout est bien envoye\n");
    }
     // Envoi d'une requete DECONNECT au serveur
    requete.expediteur = getpid();
    requete.requete = DECONNECT;
    requete.type = 1;

    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
     }
     else
    {
      printf("le DECONNECT est bien envoye\n");
    }

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogin_clicked()
{
    // Envoi d'une requete LOGIN au serveur
    requete.expediteur = getpid();
    requete.requete = LOGIN;
    requete.type = 1;
    if (isNouveauClientChecked())
    {
        requete.data1 = 1; // nouveau utilisateur est coché
    }
    else
    {
        requete.data1 = 0; // nouveau utilisateur N'EST PAS coché
    }
    strcpy(requete.data2, getNom());
    strcpy(requete.data3, getMotDePasse());


    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
     }
     else
    {
      printf("le login est bien envoye\n");
    }
    // TO DO
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonLogout_clicked()
{
    // Envoi d'une requete de logout au serveur
    // TO DO
    requete.expediteur = getpid();
    requete.requete = LOGOUT;
    requete.type = 1;
    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
    }
    else
    {
        logoutOK();
        printf("le logout est bien envoye\n");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSuivant_clicked()
{
    // TO DO (étape 3)
  requete.expediteur = getpid();
  requete.requete = CONSULT;
  requete.type = 1;
  requete.data1 = articleEnCours.id +1;
  if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
  {
    perror("Erreur de msgnd\n");
  }
  else
  {
      printf("le consult(droite) est bien envoye\n");
  }
    // Envoi d'une requete CONSULT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPrecedent_clicked()
{
    // TO DO (étape 3)
  requete.expediteur = getpid();
  requete.requete = CONSULT;
  requete.type = 1;
  requete.data1 = articleEnCours.id - 1;
  if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
  {
    perror("Erreur de msgnd\n");
  }
  else
  {
      printf("le consult(gauche) est bien envoye\n");
  }
    // Envoi d'une requete CONSULT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonAcheter_clicked()
{
    // TO DO (étape 5)
    char quantite[20]; 
    sprintf(quantite, "%d", getQuantite()); // convertir le int en char car data2 est un char[20] et getQuantite return un int

    requete.expediteur = getpid();
    requete.requete = ACHAT;
    requete.type = 1;
    requete.data1 = articleEnCours.id;
    strcpy(requete.data2, quantite);

    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
    }
    else
    {
        printf("(CLIENT)le ACHAT est bien envoye\n");
    }
    // Envoi d'une requete ACHAT au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonSupprimer_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL au serveur
    int idArticle = getIndiceArticleSelectionne();

    if(idArticle != -1)
    {
      printf("DEBUG : %d\n", idArticle);
      //requete CANCEL
      requete.expediteur = getpid();
      requete.requete = CANCEL;
      requete.type = 1;
      requete.data1 = idArticle;
      if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
      {
        perror("Erreur de msgnd\n");
      }

      // Mise à jour du caddie

      w->videTablePanier();
      totalCaddie = 0.0;
      w->setTotal(-1.0);

      //requete CADDIE

      requete.expediteur = getpid();
      requete.requete = CADDIE;
      requete.type = 1;
      if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
      {
        perror("Erreur de msgnd\n");
      }
    }
    else
    {
      dialogueErreur("Erreur de suppresion", "Pas d'article selectionne");
    }

   

    // Envoi requete CADDIE au serveur
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonViderPanier_clicked()
{
    // TO DO (étape 6)
    // Envoi d'une requete CANCEL_ALL au serveur
    requete.expediteur = getpid();
    requete.requete = CANCEL_ALL;
    requete.type = 1;
    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
    }

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    sleep(0.1); // pour laisser le temps au caddie de se vidé

    // Envoi requete CADDIE au serveur
    requete.expediteur = getpid();
    requete.requete = CADDIE;
    requete.type = 1;
    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowClient::on_pushButtonPayer_clicked()
{
    // TO DO (étape 7)
    // Envoi d'une requete PAYER au serveur
    requete.expediteur = getpid();
    requete.requete = PAYER;
    requete.type = 1;
    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
    }

    char tmp[100];
    sprintf(tmp,"Merci pour votre paiement de %.2f ! Votre commande sera livrée tout prochainement.",totalCaddie);
    dialogueMessage("Payer...",tmp);

    // Mise à jour du caddie
    w->videTablePanier();
    totalCaddie = 0.0;
    w->setTotal(-1.0);

    // Envoi requete CADDIE au serveur
    requete.expediteur = getpid();
    requete.requete = CADDIE;
    requete.type = 1;
    if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
    {
      perror("Erreur de msgnd\n");
    }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Handlers de signaux ////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void handlerSIGUSR1(int sig)
{
    
    MESSAGE m;
    char reponse[200];

    while(msgrcv(idQ,&m,sizeof(MESSAGE)-sizeof(long),getpid(),IPC_NOWAIT) != -1)  // !!! a modifier en temps voulu !!!
    {
      //fprintf(stderr, "(CLIENT)Signal %d recu\n", m.requete);
      switch(m.requete)
      {
        case LOGIN :if (m.data1 == 1) // si le log c'est bien pense
                    {
                        w->dialogueMessage("Login", m.data4);
                        w->loginOK();

                        // Envoie de la Lorsque le processus client (la fenêtre) reçoit une réponse positive après le log in, il envoie
                        //automatiquement une requête CONSULT au serveur. Cette requête contient simplement l’id
                        //(champ data1) de l’article que l’on veut consulter. Lors de cette première requête l’id doit être
                        //égal à 1
                        requete.expediteur = getpid();
                        requete.requete = CONSULT;
                        requete.type = 1;
                        requete.data1 = 1;
                        if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
                        {
                          perror("Erreur de msgnd\n");
                        }
                        else
                        {
                            printf("le consult est bien envoye\n");
                        }
                    }
                    else
                    {

                        w->dialogueErreur("Erreur de login", m.data4);
                    }
                    
                    break;

        case CONSULT : // TO DO (étape 3)
                    // mettre a jour l'article affiche
                    fprintf(stderr, "(CLIENT)Signal CONSULT recu\n");
                    articleEnCours.id = m.data1;
                    strcpy(articleEnCours.intitule, m.data2);
                    articleEnCours.prix = m.data5;
                    articleEnCours.stock = atoi(m.data3);
                    strcpy(articleEnCours.image, m.data4);
                    w->setArticle(articleEnCours.intitule,articleEnCours.prix,articleEnCours.stock,articleEnCours.image);
                    break;

        case ACHAT : // TO DO (étape 5)
                    if (atoi(m.data3)> 0) 
                    {
                      sprintf(reponse, "%s unité(s) de %s achetées avec succès", m.data3, m.data2);
                      w->dialogueMessage("Achat", reponse);

                      // envoie de la requete CADDIE au serveur pour recuperer l'entierete du caddie

                      requete.expediteur = getpid();
                      requete.requete = CADDIE;
                      if(msgsnd(idQ, &requete, sizeof(MESSAGE) - sizeof(long),0) == -1)
                      {
                        perror("Erreur de msgnd\n");
                      }
                      else
                      {
                        w->videTablePanier();
                        totalCaddie = 0;
                      }

                    }
                    else
                    {
                      w->dialogueErreur("Erreur d'Achat", "Stock insuffisant !");
                    }
                    break;

         case CADDIE : // TO DO (étape 5)
                      // pour affiche une ligne de caddie
                      w->ajouteArticleTablePanier(m.data2,m.data5,atoi(m.data3));
                      totalCaddie = totalCaddie + (m.data5 * atoi(m.data3));
                      w->setTotal(totalCaddie);
                    break;

         case TIME_OUT : // TO DO (étape 6)
                    // apres 60 secondes le client recoit un time out et est deconnecte et perd donc son caddie
                    w->logoutOK();
                    w->dialogueMessage("Time out", "Vous avez  automatiquement déconnecté pour cause d’inactivité");
                    break;

         case BUSY : // TO DO (étape 7)
                    // quand le gerant est connecte
                    w->dialogueMessage("Serveur en maintenance", "Le serveur est en maintenance");
                    break;

         default :
                    break;
      }
    }
}
void handlerSIGUSR2(int sig)
{
  w->setPublicite(pShm); // pour mettre a jour la publicite (qui decale a gauche chaque seconde)
}
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
