#ifndef FACELOGINDIALOG_H
#define FACELOGINDIALOG_H

#include <QDialog>
#include <QImage>
#include "camerathread.h"

namespace Ui {
class FaceLoginDialog;
}

class FaceLoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit FaceLoginDialog(QWidget *parent = nullptr);
    ~FaceLoginDialog();

    // For verifying face against current logged-in user
    void setExpectedUserId(int id);

signals:
    void loginSuccessful(const QString& username, const QString& role);

private slots:
    void updateFrame(const QImage &frame);
    void on_startButton_clicked();
    void on_recogniseButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::FaceLoginDialog *ui;
    CameraThread* cameraThread;
    QImage currentFrame;
    int expectedUserId = -1;  // -1 means no restriction (for login flow)

    void stopCamera();
};

#endif // FACELOGINDIALOG_H
