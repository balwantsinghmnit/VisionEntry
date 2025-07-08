#ifndef BACKUPRESTOREDIALOG_H
#define BACKUPRESTOREDIALOG_H

#include <QDialog>

namespace Ui {
class BackupRestoreDialog;
}

class BackupRestoreDialog : public QDialog {
    Q_OBJECT

public:
    explicit BackupRestoreDialog(QWidget *parent = nullptr);
    ~BackupRestoreDialog();

private slots:
    void on_backupButton_clicked();
    void on_restoreButton_clicked();
    void on_closeButton_clicked();

private:
    Ui::BackupRestoreDialog *ui;
    QString host, user, password, database;

    void loadDbConfig();
    void showMessage(const QString &text, bool success);
};

#endif // BACKUPRESTOREDIALOG_H
