#include "backuprestoredialog.h"
#include "ui_backuprestoredialog.h"

#include <QProcess>
#include <QFileDialog>
#include <QMessageBox>
#include <QSettings>
#include <QFileInfo>
#include <QDateTime>

BackupRestoreDialog::BackupRestoreDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::BackupRestoreDialog) {
    ui->setupUi(this);
    loadDbConfig();
}

BackupRestoreDialog::~BackupRestoreDialog() {
    delete ui;
}

void BackupRestoreDialog::loadDbConfig() {
    QSettings settings(QCoreApplication::applicationDirPath() + "/../../../config/config.ini", QSettings::IniFormat);
    host = settings.value("Database/Host", "localhost").toString();
    user = settings.value("Database/User", "").toString();
    password = settings.value("Database/Password", "").toString();
    database = settings.value("Database/DBName", "").toString();
}

void BackupRestoreDialog::showMessage(const QString &text, bool success) {
    if (success)
        QMessageBox::information(this, "Success", text);
    else
        QMessageBox::warning(this, "Error", text);
}

void BackupRestoreDialog::on_backupButton_clicked() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Backup File",
                                                    QDateTime::currentDateTime().toString("'backup_'yyyyMMdd_HHmmss'.sql'"),
                                                    "SQL Files (*.sql)");

    if (fileName.isEmpty())
        return;

    QStringList args = {
        "-h" + host,
        "-u" + user,
        "-p" + password,
        database
    };

    QProcess process;
    process.setProgram("mysqldump");
    process.setArguments(args);

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly)) {
        showMessage("Cannot open file for writing.", false);
        return;
    }

    process.setStandardOutputFile(fileName);
    process.start();
    process.waitForFinished();

    file.close();

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
        showMessage("Backup created successfully.", true);
    else
        showMessage("Backup failed. Ensure mysqldump is available and config is correct.", false);
}

void BackupRestoreDialog::on_restoreButton_clicked() {
    QString fileName = QFileDialog::getOpenFileName(this, "Select SQL File to Restore", "", "SQL Files (*.sql)");
    if (fileName.isEmpty())
        return;

    QStringList args = {
        "-h" + host,
        "-u" + user,
        "-p" + password,
        database
    };

    QProcess process;
    process.setProgram("mysql");
    process.setArguments(args);

    process.setStandardInputFile(fileName);
    process.start();
    process.waitForFinished();

    if (process.exitStatus() == QProcess::NormalExit && process.exitCode() == 0)
        showMessage("Database restored successfully.", true);
    else
        showMessage("Restore failed. Check SQL file and credentials.", false);
}

void BackupRestoreDialog::on_closeButton_clicked() {
    close();
}
