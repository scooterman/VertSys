#pragma once
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QtDebug>
#include <QFileInfo>

class DBManager
{
    public:
        bool openDB();
        QSqlError lastError();
        ~DBManager();

    private:
        QSqlDatabase db;
};
