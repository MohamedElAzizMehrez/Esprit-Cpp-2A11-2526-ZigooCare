#include "connection.h"

Connection::Connection() {
    db = QSqlDatabase::addDatabase("QODBC");
}
Connection::~Connection() {
    if (db.isOpen())
        db.close();
}
Connection& Connection::createInstance() {
    static Connection instance;
    return instance;
}

bool Connection::createconnect()
{bool test=false;
    db.setDatabaseName("Source_Projet2A");
    db.setUserName("ZigooCare");
    db.setPassword("azerty");

    if (db.open()){
        test=true;
       // qDebug() << "Connexion établie";
    return  test;
    }else{
       // qDebug() << "Échec de la connexion :" << db.lastError().text();
        return false;
    }
}
