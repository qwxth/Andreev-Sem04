#include "db_manager.h"
#include <QSqlError>
#include <QDebug>

DbManager& DbManager::instance() {
    static DbManager inst;
    return inst;
}

DbManager::DbManager() : connected(false) {
    db = QSqlDatabase::addDatabase("QPSQL");
    db.setDatabaseName("tv_system");
    db.setHostName("localhost");
    db.setUserName("tv_app");
    db.setPassword("tv123");
    db.setPort(5432);
}

DbManager::~DbManager() {
    if (connected) db.close();
}

bool DbManager::connect() {
    if (!db.open()) {
        qDebug() << "DB connection error:" << db.lastError().text();
        connected = false;
        return false;
    }
    connected = true;
    return true;
}

void DbManager::disconnect() {
    if (connected) db.close();
    connected = false;
}