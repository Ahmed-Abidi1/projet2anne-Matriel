#include "mainwindow.h"
#include "connection.h"
#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    Connection& c = Connection::getInstance();
    bool test = c.createConnection();

    MainWindow w;

    if (test) {
        w.show();
        QMessageBox::information(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Application démarrée avec succès!\nBase de données connectée."),
            QMessageBox::Ok
            );
    } else {
        QMessageBox::critical(
            nullptr,
            QObject::tr("Base de données"),
            QObject::tr("Échec de la connexion à la base de données.\nL'application va se fermer."),
            QMessageBox::Cancel
            );
        return -1;
    }

    return a.exec();
}
