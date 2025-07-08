#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "dbmanager.h"
#include "logger.h"
#include "mainwindow.h"
#include "facelogindialog.h"
#include "thememanager.h"  // 🔁 Theme Manager

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::LoginWindow) {
    ui->setupUi(this);
    ui->roleComboBox->addItems({"Admin", "HR", "Employee"});

    // Optional: apply default theme
    ThemeManager::instance().applyLightTheme();
}

LoginWindow::~LoginWindow() {
    delete ui;
}

void LoginWindow::on_loginButton_clicked() {
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text().trimmed();
    QString role = ui->roleComboBox->currentText().trimmed();

    int userId = -1;
    if (DBManager::instance().validateUser(username, password, role, userId)) {
        QMessageBox::information(this, "Login", "Login successful as " + role);

        Logger::instance()->log("User logged in: " + username + " [" + role + "]", Logger::INFO);

        MainWindow *dashboard = new MainWindow(username, role);
        ThemeManager::instance().applyTheme(dashboard);
        dashboard->setAttribute(Qt::WA_DeleteOnClose);
        dashboard->show();

        this->close();

        connect(dashboard, &MainWindow::destroyed, this, [=]() {
            ui->usernameLineEdit->clear();
            ui->passwordLineEdit->clear();
            ui->roleComboBox->setCurrentIndex(0);
            this->show();
        });

    } else {
        QMessageBox::warning(this, "Login Failed", "Invalid username, password, or role.");
        Logger::instance()->log("Failed login attempt for username: " + username, Logger::WARNING);
    }
}

void LoginWindow::on_faceLoginButton_clicked() {
    FaceLoginDialog dialog(this);
    connect(&dialog, &FaceLoginDialog::loginSuccessful, this, [=](const QString& username, const QString& role){
        MainWindow* dashboard = new MainWindow(username, role);
        dashboard->setAttribute(Qt::WA_DeleteOnClose);
        dashboard->show();

        this->close();

        connect(dashboard, &MainWindow::destroyed, this, [=]() {
            this->show();
        });
    });

    dialog.exec();
}

void LoginWindow::on_toggleThemeButton_clicked() {
    ThemeManager::instance().toggleTheme();
}
