#include "dbmanager.h"
#include "logger.h"
#include <QFile>
#include <QCoreApplication>

DBManager::DBManager() {}

DBManager::~DBManager() {
    disconnect();
}

DBManager& DBManager::instance() {
    static DBManager instance;
    return instance;
}

bool DBManager::loadCredentials(QString &host, QString &dbName, QString &user, QString &pass) {
    QString iniPath = QCoreApplication::applicationDirPath() + "/../../../config/config.ini";
    if (!QFile::exists(iniPath)) {
        Logger::instance()->log("Database config file not found: " + iniPath, Logger::ERROR);
        return false;
    }

    QSettings settings(iniPath, QSettings::IniFormat);
    host = settings.value("Database/Host").toString();
    dbName = settings.value("Database/DBName").toString();
    user = settings.value("Database/User").toString();
    pass = settings.value("Database/Password").toString();

    return !(host.isEmpty() || dbName.isEmpty() || user.isEmpty());
}

bool DBManager::connectToDatabase() {
    if (db.isOpen()) {
        Logger::instance()->log("Database already connected", Logger::WARNING);
        return true;
    }

    QString host, name, user, pass;
    if (!loadCredentials(host, name, user, pass)) {
        Logger::instance()->log("Failed to load database credentials", Logger::ERROR);
        return false;
    }

    db = QSqlDatabase::addDatabase("QMYSQL");
    db.setHostName(host);
    db.setDatabaseName(name);
    db.setUserName(user);
    db.setPassword(pass);

    if (!db.open()) {
        Logger::instance()->log("Database connection failed: " + db.lastError().text(), Logger::ERROR);
        return false;
    }

    Logger::instance()->log("Database connected successfully", Logger::INFO);
    return true;
}

void DBManager::disconnect() {
    if (db.isOpen()) {
        db.close();
        Logger::instance()->log("Database disconnected", Logger::INFO);
    }
}

bool DBManager::isConnected() const {
    return db.isOpen();
}

bool DBManager::validateUser(const QString& username, const QString& password, const QString& role, int& userId) {
    if (!db.isOpen()) {
        Logger::instance()->log("Database not open during login", Logger::ERROR);
        return false;
    }
    QString hashedPassword = hashPassword(password);
    QSqlQuery query(db);
    query.prepare("SELECT id FROM users WHERE username = :user AND password = :pass AND role = :role");
    query.bindValue(":user", username);
    query.bindValue(":pass", hashedPassword);
    query.bindValue(":role", role);

    if (!query.exec()) {
        Logger::instance()->log("Login query failed: " + query.lastError().text(), Logger::ERROR);
        return false;
    }

    if (query.next()) {
        userId = query.value(0).toInt();
        Logger::instance()->log("Login successful for user: " + username, Logger::INFO);
        return true;
    }

    Logger::instance()->log("Login failed for user: " + username, Logger::WARNING);
    return false;
}
