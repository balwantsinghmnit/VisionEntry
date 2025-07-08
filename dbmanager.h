#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QSettings>
#include <QCryptographicHash>

class DBManager {
public:
    static DBManager& instance();
    bool connectToDatabase();
    void disconnect();
    bool isConnected() const;
    bool validateUser(const QString& username, const QString& password, const QString& role, int& userId);

private:
    DBManager();
    ~DBManager();
    DBManager(const DBManager&) = delete;
    DBManager& operator=(const DBManager&) = delete;

    QSqlDatabase db;
    bool loadCredentials(QString &host, QString &dbName, QString &user, QString &pass);
};

static QString hashPassword(const QString& password) {
    QByteArray hashed = QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256);
    return QString(hashed.toHex());
}

#endif // DBMANAGER_H
