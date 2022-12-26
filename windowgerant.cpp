#include "windowgerant.h"
#include "ui_windowgerant.h"
#include <iostream>
using namespace std;
#include <mysql.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/sem.h>
#include "protocole.h"

int sem_wait(int num); // les deux fonctions pour les semaphores
int sem_signal(int num);

int idArticleSelectionne = -1;
MYSQL *connexion;
MYSQL_RES  *resultat;
MYSQL_ROW  Tuple;
char requete[200];
char Prix[20];
int idSem;
int idQ;
int i;
MESSAGE req;

WindowGerant::WindowGerant(QWidget *parent) : QMainWindow(parent),ui(new Ui::WindowGerant)
{
    ui->setupUi(this);

    // Configuration de la table du stock (ne pas modifer)
    ui->tableWidgetStock->setColumnCount(4);
    ui->tableWidgetStock->setRowCount(0);
    QStringList labelsTableStock;
    labelsTableStock << "Id" << "Article" << "Prix à l'unité" << "Quantité";
    ui->tableWidgetStock->setHorizontalHeaderLabels(labelsTableStock);
    ui->tableWidgetStock->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidgetStock->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidgetStock->horizontalHeader()->setVisible(true);
    ui->tableWidgetStock->horizontalHeader()->setDefaultSectionSize(120);
    ui->tableWidgetStock->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidgetStock->verticalHeader()->setVisible(false);
    ui->tableWidgetStock->horizontalHeader()->setStyleSheet("background-color: lightyellow");

    // Recuperation de la file de message
    if ((idQ = msgget(CLE,0)) == -1)
    {
      perror("Erreur de msgget");
    }

    // Récupération du sémaphore
    if ((idSem = semget(CLE,0,0)) == -1)
    {
      perror("Erreur de semget");
      exit(1);
    }
    // TO DO

    // Prise blocante du semaphore
    // TO DO
    sem_signal(0); // pour bloquer le user 

    // Connexion à la base de donnée
    connexion = mysql_init(NULL);
    fprintf(stderr,"(GERANT %d) Connexion à la BD\n",getpid());
    if (mysql_real_connect(connexion,"localhost","Student","PassStudent1_","PourStudent",0,0,0) == NULL)
    {
      fprintf(stderr,"(GERANT %d) Erreur de connexion à la base de données...\n",getpid());
      exit(1);  
    }

    // Recuperation des articles en BD
    // TO DO
    for(i=1;i<22;i++)
    {
      sprintf(requete,"select * from UNIX_FINAL where id = %d",i);
                      
      mysql_query(connexion,requete);
      resultat = mysql_store_result(connexion);
      if (resultat)
      {
        Tuple = mysql_fetch_row(resultat);

        sprintf(Prix,"%s",Tuple[2]);
        string tmp(Prix);
        size_t x = tmp.find(".");
        if (x != string::npos) tmp.replace(x,1,",");  
        strcpy(Prix, tmp.data());

        printf("ARGENT : %s\n", (Prix)); // ne veut pas floater
        ajouteArticleTablePanier(atoi(Tuple[0]),Tuple[1],atof(Prix),atoi(Tuple[3]));
      }
    }
    
}

WindowGerant::~WindowGerant()
{
    delete ui;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles Table du stock (ne pas modifier) //////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::ajouteArticleTablePanier(int id,const char* article,float prix,int quantite)
{
    char Id[20],Prix[20],Quantite[20];

    sprintf(Id,"%d",id);
    sprintf(Prix,"%.2f",prix);
    sprintf(Quantite,"%d",quantite);

    // Ajout possible
    int nbLignes = ui->tableWidgetStock->rowCount();
    nbLignes++;
    ui->tableWidgetStock->setRowCount(nbLignes);
    ui->tableWidgetStock->setRowHeight(nbLignes-1,10);

    QTableWidgetItem *item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Id);
    ui->tableWidgetStock->setItem(nbLignes-1,0,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(article);
    ui->tableWidgetStock->setItem(nbLignes-1,1,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Prix);
    ui->tableWidgetStock->setItem(nbLignes-1,2,item);

    item = new QTableWidgetItem;
    item->setFlags(Qt::ItemIsSelectable|Qt::ItemIsEnabled);
    item->setTextAlignment(Qt::AlignCenter);
    item->setText(Quantite);
    ui->tableWidgetStock->setItem(nbLignes-1,3,item);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::videTableStock()
{
    ui->tableWidgetStock->setRowCount(0);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowGerant::getIndiceArticleSelectionne()
{
    QModelIndexList liste = ui->tableWidgetStock->selectionModel()->selectedRows();
    if (liste.size() == 0) return -1;
    QModelIndex index = liste.at(0);
    int indice = index.row();
    return indice;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_tableWidgetStock_cellClicked(int row, int column)
{
    //cerr << "ligne=" << row << " colonne=" << column << endl;
    ui->lineEditIntitule->setText(ui->tableWidgetStock->item(row,1)->text());
    ui->lineEditPrix->setText(ui->tableWidgetStock->item(row,2)->text());
    ui->lineEditStock->setText(ui->tableWidgetStock->item(row,3)->text());
    idArticleSelectionne = atoi(ui->tableWidgetStock->item(row,0)->text().toStdString().c_str());
    //cerr << "id = " << idArticleSelectionne << endl;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions utiles : ne pas modifier /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
float WindowGerant::getPrix()
{
    return atof(ui->lineEditPrix->text().toStdString().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
int WindowGerant::getStock()
{
    return atoi(ui->lineEditStock->text().toStdString().c_str());
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
const char* WindowGerant::getPublicite()
{
  strcpy(publicite,ui->lineEditPublicite->text().toStdString().c_str());
  return publicite;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////// CLIC SUR LA CROIX DE LA FENETRE /////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::closeEvent(QCloseEvent *event)
{
  fprintf(stderr,"(GERANT %d) Clic sur croix de la fenetre\n",getpid());
  // TO DO
  // Deconnexion BD
  mysql_close(connexion);

  // Liberation du semaphore
  sem_signal(0); // pour débloquer le user

  // TO DO

  exit(0);
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////
///// Fonctions clics sur les boutons ////////////////////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_pushButtonPublicite_clicked()
{
  fprintf(stderr,"(GERANT %d) Clic sur bouton Mettre a jour\n",getpid());
  // TO DO (étape 7)
  // Envoi d'une requete NEW_PUB au serveur
  req.expediteur = getpid();
  req.requete = NEW_PUB;
  req.type = 1;
  strcpy(req.data4, getPublicite());
  if(msgsnd(idQ, &req, sizeof(MESSAGE) - sizeof(long),0) == -1)
  {
    perror("Erreur de msgnd\n");
  }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////
void WindowGerant::on_pushButtonModifier_clicked()
{
  fprintf(stderr,"(GERANT %d) Clic sur bouton Modifier\n",getpid());
  // TO DO
  //cerr << "Prix  : --"  << getPrix() << "--" << endl;
  //cerr << "Stock : --"  << getStock() << "--" << endl;

  sprintf(Prix,"%f",getPrix());
  string tmp(Prix);
  size_t x = tmp.find(",");
  if (x != string::npos) tmp.replace(x,1,".");
  strcpy(Prix, tmp.data());
  printf("%f\n", getPrix());

  sprintf(requete, "update UNIX_FINAL SET stock = %d where id = %d",getStock(),idArticleSelectionne);
  mysql_query(connexion,requete);
  sprintf(requete, "update UNIX_FINAL SET prix = %s where id = %d",Prix,idArticleSelectionne);
  mysql_query(connexion,requete);
  fprintf(stderr,"(GERANT %d) Modification en base de données pour id=%d\n",getpid(),idArticleSelectionne);

  // Mise a jour table BD
  videTableStock();
  for(i=1;i<22;i++)
    {
      sprintf(requete,"select * from UNIX_FINAL where id = %d",i);
                      
      mysql_query(connexion,requete);
      resultat = mysql_store_result(connexion);
      if (resultat)
      {
        Tuple = mysql_fetch_row(resultat); 

        sprintf(Prix,"%s",Tuple[2]);
        string tmp(Prix);
        size_t x = tmp.find(".");
        if (x != string::npos) tmp.replace(x,1,",");  
        strcpy(Prix, tmp.data());

        ajouteArticleTablePanier(atoi(Tuple[0]),Tuple[1],atof(Prix),atoi(Tuple[3]));
      }
    }
  // TO DO
}


int sem_wait(int num)
{
  struct sembuf action;
  action.sem_num = num;
  action.sem_op = -1;
  action.sem_flg = SEM_UNDO;
  return semop(idSem,&action,1);
}
int sem_signal(int num)
{
  struct sembuf action;
  action.sem_num = num;
  action.sem_op = +1;
  action.sem_flg = SEM_UNDO;
  return semop(idSem,&action,1);
}
