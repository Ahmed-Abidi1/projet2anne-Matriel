#ifndef MATERIEL_H
#define MATERIEL_H

#include <QString>
#include <QSqlQuery>
#include <QTableWidget>
#include <QDebug>
#include <QSqlError>

class Materiel
{
    QString code_barre;
    QString nom_matr;
    QString type_mat;
    QString etat;
    int qtite;

public:
    Materiel();
    Materiel(QString, QString, QString, QString, int);

    QString getCodeBarre() { return code_barre; }
    QString getNomMatr() { return nom_matr; }
    QString getTypeMat() { return type_mat; }
    QString getEtat() { return etat; }
    int getQtite() { return qtite; }

    void setCodeBarre(QString c) { code_barre = c; }
    void setNomMatr(QString n) { nom_matr = n; }
    void setTypeMat(QString t) { type_mat = t; }
    void setEtat(QString e) { etat = e; }
    void setQtite(int q) { qtite = q; }

    bool ajouter();
    void afficher(QTableWidget *table);
    bool supprimer(QString code_barre);
    bool modifier(QString code_barre);
    void rechercher(QTableWidget *table, QString code_barre);
    void trier(QTableWidget *table, QString critere);
    bool existe(QString code_barre);
};

#endif // MATERIEL_H
