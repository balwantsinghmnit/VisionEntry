#include "facelogindialog.h"
#include "ui_facelogindialog.h"
#include "logger.h"
#include "dbmanager.h"
#include "mainwindow.h"

#include <QMessageBox>
#include <QProcess>
#include <QSqlQuery>
#include <QSqlError>
#include <opencv2/opencv.hpp>

FaceLoginDialog::FaceLoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::FaceLoginDialog)
    , cameraThread(new CameraThread(this))
{
    ui->setupUi(this);
    connect(cameraThread, &CameraThread::frameCaptured, this, &FaceLoginDialog::updateFrame);
}

FaceLoginDialog::~FaceLoginDialog()
{
    stopCamera();
    delete ui;
}

void FaceLoginDialog::on_startButton_clicked()
{
    if (!cameraThread->isRunning()) {
        cameraThread->start();
        Logger::instance()->log("FaceLoginDialog: Camera started", Logger::INFO);
    }
}

void FaceLoginDialog::updateFrame(const QImage &frame)
{
    if (!frame.isNull()) {
        currentFrame = frame.copy();
        ui->cameraLabel->setPixmap(QPixmap::fromImage(currentFrame).scaled(
            ui->cameraLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));
    }
}

void FaceLoginDialog::on_recogniseButton_clicked()
{
    if (currentFrame.isNull()) {
        QMessageBox::warning(this, "No Frame", "No camera frame to capture.");
        return;
    }
    // Save captured frame to disk
    cv::Mat captured = cv::Mat(currentFrame.height(), currentFrame.width(), CV_8UC3,
                               const_cast<uchar*>(currentFrame.bits()),
                               currentFrame.bytesPerLine()).clone();
    cv::imwrite(QCoreApplication::applicationDirPath().toStdString() + "/../../../python_scripts/recognise_frame.jpg", captured);

    // Call Python recognition script
    QProcess process;
    QString python = "python";
    QStringList args = {QCoreApplication::applicationDirPath() + "/../../../python_scripts/face_recognise.py", QCoreApplication::applicationDirPath() + "/../../../python_scripts/recognise_frame.jpg"};
    process.start(python, args);
    process.waitForFinished();

    QString output = process.readAllStandardOutput().trimmed();

    if (output.startsWith("ERROR") || output.isEmpty()) {
        QMessageBox::warning(this, "Recognition Failed", "No face matched.");
        return;
    }

    int userId = output.toInt();

    // If expectedUserId is set, ensure face matches logged-in account
    if (expectedUserId != -1 && userId != expectedUserId) {
        QMessageBox::warning(this, "Face Mismatch", "This face does not match your logged-in account.");
        Logger::instance()->log("FaceLoginDialog: Face mismatch. Expected user ID " + QString::number(expectedUserId) +
                                    ", but got " + QString::number(userId), Logger::WARNING);
        return;
    }

    // Fetch username and role from database
    QSqlQuery query;
    query.prepare("SELECT username, role FROM users WHERE id = ?");
    query.addBindValue(userId);

    if (!query.exec() || !query.next()) {
        QMessageBox::warning(this, "User Not Found", "No matching user found in database.");
        Logger::instance()->log("FaceLogin: No user for id " + QString::number(userId), Logger::ERROR);
        return;
    }

    QString username = query.value("username").toString();
    QString role = query.value("role").toString();

    // Check if attendance is already marked for today
    QSqlQuery checkQuery;
    checkQuery.prepare("SELECT COUNT(*) FROM attendance WHERE user_id = ? AND DATE(timestamp) = CURDATE()");
    checkQuery.addBindValue(userId);
    bool alreadyMarked = false;

    if (!checkQuery.exec()) {
        Logger::instance()->log("Attendance check failed: " + checkQuery.lastError().text(), Logger::ERROR);
    } else if (checkQuery.next() && checkQuery.value(0).toInt() > 0) {
        alreadyMarked = true;
    }

    // Insert attendance if not already marked
    if (!alreadyMarked) {
        QSqlQuery mark;
        mark.prepare("INSERT INTO attendance (user_id, status) VALUES (?, ?)");
        mark.addBindValue(userId);
        mark.addBindValue("Present");

        if (!mark.exec()) {
            Logger::instance()->log("Attendance insert failed: " + mark.lastError().text(), Logger::ERROR);
        } else {
            Logger::instance()->log("Attendance marked for " + username, Logger::INFO);
        }
    }

    // Emit login success and close dialog
    emit loginSuccessful(username, role);
    accept();
}

void FaceLoginDialog::on_cancelButton_clicked()
{
    reject();
}

void FaceLoginDialog::stopCamera()
{
    if (cameraThread) {
        cameraThread->stop();
        cameraThread->wait();
        delete cameraThread;
        cameraThread = nullptr;
    }
}

void FaceLoginDialog::setExpectedUserId(int id)
{
    expectedUserId = id;
}
