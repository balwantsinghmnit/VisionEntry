#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include <QImage>
#include <QTimer>
#include <QByteArray>
#include <opencv2/opencv.hpp>

#include "camerathread.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog {
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    ~RegisterDialog();

private slots:
    void updateFrame(const QImage &frame);
    void on_captureButton_clicked();
    void on_saveButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::RegisterDialog *ui;
    CameraThread *cameraThread;
    QImage currentFrame;
    cv::Mat capturedFace;
    QByteArray encodeFace(const cv::Mat &face);
    void stopCamera();
    bool isUsernameTaken(const QString &username);
    bool insertUser(const QString &username, const QString &hashedPassword, const QString &role,
                    const QString &fullName, int &userIdOut);
    bool insertFaceData(int userId, const QByteArray &encoding);
};

#endif // REGISTERDIALOG_H
