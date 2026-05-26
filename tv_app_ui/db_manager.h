#ifndef DB_MANAGER_H
#define DB_MANAGER_H

#include <QSqlDatabase>

class DbManager {
public:
    static DbManager& instance();
    bool connect();
    void disconnect();
    QSqlDatabase getDatabase() const { return db; }

private:
    DbManager();
    ~DbManager();
    QSqlDatabase db;
    bool connected;
};

#endif // DB_MANAGER_H