#include <QApplication>
#include <QDebug>
#include "logger.h"
#include "dbmanager.h"
#include "loginwindow.h"

int main(int argc, char *argv[]) {
    QApplication a(argc, argv);

    Logger::instance()->log("Application started", Logger::INFO);

    // Connecting database
    if (!DBManager::instance().connectToDatabase()) {
        return -1;
    }

    LoginWindow w;
    w.show();

    return a.exec();
}
