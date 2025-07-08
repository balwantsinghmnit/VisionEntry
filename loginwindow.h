#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>

namespace Ui {
class LoginWindow;
}

class LoginWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

signals:
    void loginSuccess(const QString &username, const QString &role);

private slots:
    void on_loginButton_clicked();
    void on_faceLoginButton_clicked();
    void on_toggleThemeButton_clicked();  // NEW

private:
    Ui::LoginWindow *ui;
};

#endif // LOGINWINDOW_H
