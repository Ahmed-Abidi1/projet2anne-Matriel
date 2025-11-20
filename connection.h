#ifndef CONNECTION_H
#define CONNECTION_H

#include <QSqlDatabase>
#include <QDebug>
#include <QSqlError>
#include <QMessageBox>
#include <QString>
#include <QSqlQuery>

class Connection
{
public:
    static Connection& getInstance();
    bool createConnection();
    QSqlDatabase getDatabase() { return db; }
    bool migrateSQLiteToOracle();

private:
    QSqlDatabase db;

    // Add these method declarations
    void checkODBCDrivers();
    bool setupSQLite();

    Connection();
    ~Connection();
    Connection(const Connection&) = delete;
    Connection& operator=(const Connection&) = delete;
};

#endif // CONNECTION_H
