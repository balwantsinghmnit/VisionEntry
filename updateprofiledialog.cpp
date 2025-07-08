#include "updateprofiledialog.h"
#include "ui_updateprofiledialog.h"
#include "dbmanager.h"
#include "logger.h"

#include <QMessageBox>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QProcess>
#include <QBuffer>

UpdateProfileDialog::UpdateProfileDialog(int userId, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::UpdateProfileDialog)
    , userId(userId)
{
    ui->setupUi(this);
    loadUserData();

    cameraThread = new CameraThread(this);
    connect(cameraThread, &CameraThread::frameCaptured, this, [=](const QImage &frame) {
        currentFrame = frame.copy();
        ui->cameraLabel->setPixmap(QPixmap::fromImage(frame).scaled(
            ui->cameraLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation
            ));
    });
    cameraThread->start();
    Logger::instance()->log("Camera started in UpdateProfileDialog", Logger::INFO);
}

UpdateProfileDialog::~UpdateProfileDialog()
{
    stopCamera();
    delete ui;
}

void UpdateProfileDialog::stopCamera()
{
    if (cameraThread) {
        cameraThread->stop();
        cameraThread->wait();
        delete cameraThread;
        cameraThread = nullptr;
    }
}

void UpdateProfileDialog::loadUserData()
{
    QSqlQuery query;
    query.prepare("SELECT full_name, role FROM users WHERE id = ?");
    query.addBindValue(userId);

    if (query.exec() && query.next()) {
        ui->fullNameLineEdit->setText(query.value(0).toString());
        ui->roleLineEdit->setText(query.value(1).toString());
        ui->roleLineEdit->setDisabled(true);
    } else {
        showMessage("Failed to load user data.", true);
    }
}

void UpdateProfileDialog::on_updateInfoButton_clicked()
{
    QString fullName = ui->fullNameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();

    if (fullName.isEmpty()) {
        showMessage("Full name cannot be empty.", true);
        return;
    }

    QSqlQuery query;
    if (password.isEmpty()) {
        query.prepare("UPDATE users SET full_name = ? WHERE id = ?");
        query.addBindValue(fullName);
        query.addBindValue(userId);
    } else {
        QString hashedPass = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());
        query.prepare("UPDATE users SET full_name = ?, password = ? WHERE id = ?");
        query.addBindValue(fullName);
        query.addBindValue(hashedPass);
        query.addBindValue(userId);
    }

    if (query.exec()) {
        showMessage("Profile updated successfully.");
        Logger::instance()->log("Profile updated for user id: " + QString::number(userId), Logger::INFO);
    } else {
        showMessage("Failed to update profile.", true);
        Logger::instance()->log("Profile update failed: " + query.lastError().text(), Logger::ERROR);
    }
}

void UpdateProfileDialog::on_captureFaceButton_clicked()
{
    if (currentFrame.isNull()) {
        showMessage("No camera frame available. Try again.", true);
        return;
    }

    capturedFace = cv::Mat(currentFrame.height(), currentFrame.width(), CV_8UC3,
                           const_cast<uchar*>(currentFrame.bits()),
                           currentFrame.bytesPerLine()).clone();

    cv::imwrite("captured_update.jpg", capturedFace);
    showMessage("Face captured. Now click Update Face.");
}

void UpdateProfileDialog::on_updateFaceButton_clicked()
{
    if (capturedFace.empty()) {
        showMessage("Please capture a face image first.", true);
        return;
    }

    QString pythonPath = "python";
    QStringList args;
    args << QCoreApplication::applicationDirPath() + "/../../../python_scripts/face_register.py" << QCoreApplication::applicationDirPath() + "/../../../python_scripts/captured_update.jpg";

    QProcess process;
    process.start(pythonPath, args);
    process.waitForFinished();

    QString output = process.readAllStandardOutput().trimmed();

    if (output.startsWith("ERROR")) {
        showMessage(output, true);
        return;
    }

    QByteArray encoded = QByteArray::fromBase64(output.toUtf8());

    QSqlQuery query;
    query.prepare("UPDATE face_data SET encoding = ? WHERE user_id = ?");
    query.addBindValue(encoded);
    query.addBindValue(userId);

    if (query.exec()) {
        showMessage("Face data updated successfully.");
        Logger::instance()->log("Face encoding updated for user " + QString::number(userId), Logger::INFO);
    } else {
        showMessage("Failed to update face data.", true);
        Logger::instance()->log("Face encoding update failed: " + query.lastError().text(), Logger::ERROR);
    }
}

void UpdateProfileDialog::showMessage(const QString &text, bool error)
{
    if (error)
        QMessageBox::warning(this, "Error", text);
    else
        QMessageBox::information(this, "Success", text);
}

void UpdateProfileDialog::on_buttonBoxOk_clicked()
{
    accept();
}


void UpdateProfileDialog::on_buttonBoxCancel_clicked()
{
    reject();
}

