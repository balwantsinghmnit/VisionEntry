#ifndef UPDATEPROFILEDIALOG_H
#define UPDATEPROFILEDIALOG_H

#include <QDialog>
#include <QImage>
#include <opencv2/opencv.hpp>
#include "camerathread.h"

namespace Ui {
class UpdateProfileDialog;
}

class UpdateProfileDialog : public QDialog {
    Q_OBJECT

public:
    explicit UpdateProfileDialog(int userId, QWidget *parent = nullptr);
    ~UpdateProfileDialog();

private slots:
    void on_updateInfoButton_clicked();
    void on_updateFaceButton_clicked();
    void on_captureFaceButton_clicked();

    void on_buttonBoxOk_clicked();

    void on_buttonBoxCancel_clicked();

private:
    void loadUserData();
    void showMessage(const QString &text, bool error = false);
    void stopCamera();

    Ui::UpdateProfileDialog *ui;
    int userId;

    CameraThread *cameraThread = nullptr;
    QImage currentFrame;
    cv::Mat capturedFace;
};

#endif // UPDATEPROFILEDIALOG_H
