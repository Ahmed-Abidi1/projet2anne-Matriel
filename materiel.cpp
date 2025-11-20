#include "materiel.h"
#include <QTableWidget>
#include <QHeaderView>
#include <QDebug>

Materiel::Materiel()
{
    code_barre = "";
    nom_matr = "";
    type_mat = "";
    etat = "";
    qtite = 0;
}

Materiel::Materiel(QString code_barre, QString nom_matr, QString type_mat, QString etat, int qtite)
{
    this->code_barre = code_barre;
    this->nom_matr = nom_matr;
    this->type_mat = type_mat;
    this->etat = etat;
    this->qtite = qtite;
}

bool Materiel::ajouter()
{
    // Check database connection
    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isOpen()) {
        qDebug() << "ERREUR: Base de données non connectée!";
        return false;
    }

    qDebug() << "=== TENTATIVE D'AJOUT ===";
    qDebug() << "Type de base:" << db.driverName();
    qDebug() << "Données: Code Barre:" << code_barre << "Nom:" << nom_matr;

    QSqlQuery query;

    query.prepare("INSERT INTO MATERIEL (CODE_BARRE, NOM_MATR, TYPE_MAT, ETAT, QTITE) "
                  "VALUES (:code_barre, :nom_matr, :type_mat, :etat, :qtite)");

    query.bindValue(":code_barre", code_barre);
    query.bindValue(":nom_matr", nom_matr);
    query.bindValue(":type_mat", type_mat);
    query.bindValue(":etat", etat);
    query.bindValue(":qtite", qtite);

    bool success = query.exec();

    if (success) {
        qDebug() << "SUCCÈS: Matériel ajouté!";
        return true;
    } else {
        qDebug() << "ERREUR SQL: Impossible d'ajouter le matériel";
        return false;
    }
}

void Materiel::afficher(QTableWidget *table)
{
    QSqlQuery query;
    bool success = query.exec("SELECT CODE_BARRE, NOM_MATR, TYPE_MAT, ETAT, QTITE FROM MATERIEL ORDER BY NOM_MATR");

    if (!success) {
        qDebug() << "Erreur affichage";
        return;
    }

    table->setRowCount(0);

    QStringList headers;
    headers << "Code Barre" << "Nom" << "Type" << "Etat" << "Quantité";
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels(headers);

    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        row++;
    }

    qDebug() << "Affichage: " << row << "lignes trouvées";
    table->horizontalHeader()->setStretchLastSection(true);
}

bool Materiel::supprimer(QString code_barre)
{
    QSqlQuery query;

    query.prepare("DELETE FROM MATERIEL WHERE CODE_BARRE = :code_barre");
    query.bindValue(":code_barre", code_barre);

    return query.exec();
}

bool Materiel::modifier(QString code_barre)
{
    QSqlQuery query;

    // Build the update query dynamically based on which fields are set
    QString queryStr = "UPDATE MATERIEL SET ";
    bool firstField = true;

    if (!nom_matr.isEmpty()) {
        queryStr += "NOM_MATR = :nom_matr";
        firstField = false;
    }

    if (!type_mat.isEmpty()) {
        if (!firstField) queryStr += ", ";
        queryStr += "TYPE_MAT = :type_mat";
        firstField = false;
    }

    if (!etat.isEmpty()) {
        if (!firstField) queryStr += ", ";
        queryStr += "ETAT = :etat";
        firstField = false;
    }

    if (qtite != 0) { // Only update quantity if it's not 0 (default)
        if (!firstField) queryStr += ", ";
        queryStr += "QTITE = :qtite";
    }

    queryStr += " WHERE CODE_BARRE = :code_barre";

    query.prepare(queryStr);

    // Only bind values that are being updated
    if (!nom_matr.isEmpty()) {
        query.bindValue(":nom_matr", nom_matr);
    }
    if (!type_mat.isEmpty()) {
        query.bindValue(":type_mat", type_mat);
    }
    if (!etat.isEmpty()) {
        query.bindValue(":etat", etat);
    }
    if (qtite != 0) {
        query.bindValue(":qtite", qtite);
    }

    query.bindValue(":code_barre", code_barre);

    bool success = query.exec();

    if (success) {
        qDebug() << "SUCCÈS: Matériel modifié! Code barre:" << code_barre;
        qDebug() << "Query executed:" << queryStr;
    } else {
        qDebug() << "ERREUR SQL: Impossible de modifier le matériel:" << query.lastError().text();
    }

    return success;
}

void Materiel::rechercher(QTableWidget *table, QString code_barre)
{
    QSqlQuery query;
    query.prepare("SELECT CODE_BARRE, NOM_MATR, TYPE_MAT, ETAT, QTITE FROM MATERIEL "
                  "WHERE UPPER(CODE_BARRE) LIKE UPPER(:code_barre)");
    query.bindValue(":code_barre", "%" + code_barre + "%");

    if (!query.exec()) {
        qDebug() << "Recherche error:" << query.lastError().text();
        return;
    }

    table->setRowCount(0);

    QStringList headers;
    headers << "Code Barre" << "Nom" << "Type" << "Etat" << "Quantité";
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels(headers);

    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        row++;
    }

    table->horizontalHeader()->setStretchLastSection(true);
    qDebug() << "Recherche found:" << row << "results for code barre:" << code_barre;
}

void Materiel::trier(QTableWidget *table, QString critere)
{
    QString queryStr = "SELECT CODE_BARRE, NOM_MATR, TYPE_MAT, ETAT, QTITE FROM MATERIEL ORDER BY ";

    if (critere == "nom") queryStr += "NOM_MATR";
    else if (critere == "Type") queryStr += "TYPE_MAT";
    else if (critere == "Etat") queryStr += "ETAT";
    else if (critere == "Quantité") queryStr += "QTITE";
    else queryStr += "NOM_MATR";

    QSqlQuery query;
    if (!query.exec(queryStr)) {
        return;
    }

    table->setRowCount(0);

    QStringList headers;
    headers << "Code Barre" << "Nom" << "Type" << "Etat" << "Quantité";
    table->setColumnCount(5);
    table->setHorizontalHeaderLabels(headers);

    int row = 0;
    while (query.next()) {
        table->insertRow(row);
        table->setItem(row, 0, new QTableWidgetItem(query.value(0).toString()));
        table->setItem(row, 1, new QTableWidgetItem(query.value(1).toString()));
        table->setItem(row, 2, new QTableWidgetItem(query.value(2).toString()));
        table->setItem(row, 3, new QTableWidgetItem(query.value(3).toString()));
        table->setItem(row, 4, new QTableWidgetItem(query.value(4).toString()));
        row++;
    }

    table->horizontalHeader()->setStretchLastSection(true);
}

bool Materiel::existe(QString code_barre)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM MATERIEL WHERE CODE_BARRE = :code_barre");
    query.bindValue(":code_barre", code_barre);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }
    return false;
}
