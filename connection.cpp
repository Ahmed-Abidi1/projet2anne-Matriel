#include "connection.h"
#include <QDir>
#include <QSqlQuery>
#include <QStringList>
#include <QProcess>
#include <QSettings>
#include <QFile>

Connection::Connection()
{
    db = QSqlDatabase::addDatabase("QODBC");
}

Connection::~Connection()
{
    if (db.isOpen()) {
        db.close();
    }
}

Connection& Connection::getInstance()
{
    static Connection instance;
    return instance;
}

bool Connection::createConnection()
{
    qDebug() << "=== CONNECTING TO ORACLE XE ===";

    // Use the exact driver name we found: "Oracle in XE"
    QStringList xeConnections = {
        "DRIVER={Oracle in XE};DBQ=localhost:1521/XE;UID=SOUMAYA;PWD=esprit18",
        "DRIVER={Oracle in XE};DBQ=XE;UID=SOUMAYA;PWD=esprit18",
        "DRIVER={Oracle in XE};SERVER=localhost:1521/XE;UID=SOUMAYA;PWD=esprit18",
        "DRIVER={Oracle in XE};DBQ=(DESCRIPTION=(ADDRESS=(PROTOCOL=TCP)(HOST=localhost)(PORT=1521))(CONNECT_DATA=(SERVICE_NAME=XE)));UID=SOUMAYA;PWD=esprit18"
    };

    for (const QString &connStr : xeConnections) {
        qDebug() << "Trying:" << connStr;
        db.setDatabaseName(connStr);

        if (db.open()) {
            qDebug() << "✅ ORACLE XE CONNECTION SUCCESSFUL!";

            // Test the connection
            QSqlQuery query;
            if (query.exec("SELECT USER FROM DUAL")) {
                query.next();
                qDebug() << "Connected as user:" << query.value(0).toString();
            }

            // Create MATERIEL table if it doesn't exist
            QString createTable =
                "BEGIN "
                "   EXECUTE IMMEDIATE 'CREATE TABLE MATERIEL ("
                "       CODE_BARRE   VARCHAR2(100) PRIMARY KEY,"
                "       NOM_MATR     VARCHAR2(100) NOT NULL,"
                "       TYPE_MAT     VARCHAR2(100),"
                "       ETAT         VARCHAR2(50),"
                "       QTITE        NUMBER DEFAULT 0)';"
                "EXCEPTION "
                "   WHEN OTHERS THEN NULL;"
                "END;";

            QSqlQuery createQuery;
            if (createQuery.exec(createTable)) {
                qDebug() << "MATERIEL table ready";
            }

            return true;
        } else {
            qDebug() << "Failed:" << db.lastError().text();
        }
    }

    // Try DSN connection
    qDebug() << "\nTrying DSN connection...";
    db.setDatabaseName("DSN=Source_Projet2A;UID=SOUMAYA;PWD=esprit18");
    if (db.open()) {
        qDebug() << "✅ DSN CONNECTION SUCCESSFUL!";
        return true;
    } else {
        qDebug() << "DSN failed:" << db.lastError().text();
    }

    // Fallback to SQLite
    qDebug() << "\nFalling back to SQLite...";
    return setupSQLite();
}

void Connection::checkODBCDrivers()
{
    qDebug() << "Checking installed ODBC drivers...";

    // Check 64-bit ODBC drivers
    QSettings registry64("HKEY_LOCAL_MACHINE\\SOFTWARE\\ODBC\\ODBCINST.INI\\ODBC Drivers", QSettings::NativeFormat);
    QStringList drivers64 = registry64.childKeys();

    qDebug() << "64-bit ODBC Drivers:";
    if (drivers64.isEmpty()) {
        qDebug() << "  None found";
    } else {
        foreach(QString driver, drivers64) {
            qDebug() << "  -" << driver;
        }
    }

    // Also check 32-bit drivers (might be in different registry path)
    QSettings registry32("HKEY_LOCAL_MACHINE\\SOFTWARE\\WOW6432Node\\ODBC\\ODBCINST.INI\\ODBC Drivers", QSettings::NativeFormat);
    QStringList drivers32 = registry32.childKeys();

    if (!drivers32.isEmpty()) {
        qDebug() << "32-bit ODBC Drivers:";
        foreach(QString driver, drivers32) {
            qDebug() << "  -" << driver;
        }
    }
}

bool Connection::setupSQLite()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    QString dbPath = QDir::currentPath() + "/materiel_temp.db";
    db.setDatabaseName(dbPath);

    if (db.open()) {
        QSqlQuery query;
        query.exec("CREATE TABLE IF NOT EXISTS MATERIEL ("
                   "CODE_BARRE   VARCHAR(100) PRIMARY KEY,"
                   "NOM_MATR     VARCHAR(100) NOT NULL,"
                   "TYPE_MAT     VARCHAR(100),"
                   "ETAT         VARCHAR(50),"
                   "QTITE        INTEGER DEFAULT 0)");

        QMessageBox::information(nullptr, "SQLite Mode",
                                 "Oracle connection failed.\n"
                                 "Using SQLite temporarily.\n"
                                 "Data will be migrated to Oracle when available.");
        return true;
    }

    return false;
}

bool Connection::migrateSQLiteToOracle()
{
    // Check if there's SQLite data to migrate
    QSqlDatabase sqliteDb = QSqlDatabase::addDatabase("QSQLITE", "migration_connection");
    QString sqlitePath = QDir::currentPath() + "/materiel_temp.db";

    if (!QFile::exists(sqlitePath)) {
        return true; // No data to migrate
    }

    sqliteDb.setDatabaseName(sqlitePath);
    if (!sqliteDb.open()) {
        return false;
    }

    // Read data from SQLite
    QSqlQuery sqliteQuery("SELECT * FROM MATERIEL", sqliteDb);
    int migratedCount = 0;

    while (sqliteQuery.next()) {
        QString code_barre = sqliteQuery.value(0).toString();
        QString nom_matr = sqliteQuery.value(1).toString();
        QString type_mat = sqliteQuery.value(2).toString();
        QString etat = sqliteQuery.value(3).toString();
        int qtite = sqliteQuery.value(4).toInt();

        // Insert into Oracle
        QSqlQuery oracleQuery;
        oracleQuery.prepare("INSERT INTO MATERIEL (CODE_BARRE, NOM_MATR, TYPE_MAT, ETAT, QTITE) "
                            "VALUES (:code_barre, :nom_matr, :type_mat, :etat, :qtite)");
        oracleQuery.bindValue(":code_barre", code_barre);
        oracleQuery.bindValue(":nom_matr", nom_matr);
        oracleQuery.bindValue(":type_mat", type_mat);
        oracleQuery.bindValue(":etat", etat);
        oracleQuery.bindValue(":qtite", qtite);

        if (oracleQuery.exec()) {
            migratedCount++;
        }
    }

    sqliteDb.close();

    if (migratedCount > 0) {
        qDebug() << "✅" << migratedCount << "records migrated from SQLite to Oracle";
        QFile::remove(sqlitePath); // Remove temporary SQLite file
    }

    return true;
}
