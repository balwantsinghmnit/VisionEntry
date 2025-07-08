#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "logger.h"
#include "dbmanager.h"

#include <QMessageBox>
#include <QBuffer>
#include <QSqlQuery>
#include <QSqlError>
#include <QCryptographicHash>
#include <QProcess>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
    , cameraThread(new CameraThread(this)) {

    ui->setupUi(this);
    ui->cameraLabel->setStyleSheet("border: 2px solid gray; border-radius: 5px;");
    connect(cameraThread, &CameraThread::frameCaptured, this, &RegisterDialog::updateFrame);
    cameraThread->start();

    Logger::instance()->log("RegisterDialog camera started", Logger::INFO);
}

RegisterDialog::~RegisterDialog() {
    stopCamera();
    delete ui;
}

void RegisterDialog::updateFrame(const QImage &frame) {
    if (!frame.isNull() && ui && ui->cameraLabel) {
        currentFrame = frame.copy();
        ui->cameraLabel->setPixmap(QPixmap::fromImage(currentFrame).scaled(
            ui->cameraLabel->size(),
            Qt::KeepAspectRatio,
            Qt::SmoothTransformation));
    }
}

void RegisterDialog::on_captureButton_clicked() {
    if (currentFrame.isNull()) {
        QMessageBox::warning(this, "Capture Failed", "No camera frame available.");
        return;
    }

    capturedFace = cv::Mat(currentFrame.height(), currentFrame.width(), CV_8UC3,
                           const_cast<uchar*>(currentFrame.bits()),
                           currentFrame.bytesPerLine()).clone();
    if (!cv::imwrite(QCoreApplication::applicationDirPath().toStdString() + "/../../../python_scripts/captured_frame.jpg", capturedFace)) {
        QMessageBox::warning(this, "Capture Error", "Failed to save captured frame.");
        Logger::instance()->log("cv::imwrite failed to save image", Logger::ERROR);
        return;
    }

    QMessageBox::information(this, "Capture Successful", "Frame captured. Ready to save.");
}

QByteArray RegisterDialog::encodeFace(const cv::Mat &face) {
    std::vector<uchar> buf;
    std::vector<int> params = {cv::IMWRITE_JPEG_QUALITY, 90};
    cv::imencode(".jpg", face, buf, params);
    return QByteArray(reinterpret_cast<const char*>(buf.data()), static_cast<int>(buf.size()));
}

bool RegisterDialog::isUsernameTaken(const QString &username) {
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM users WHERE username = ?");
    query.addBindValue(username);
    return query.exec() && query.next() && query.value(0).toInt() > 0;
}

bool RegisterDialog::insertUser(const QString &username, const QString &hashedPassword, const QString &role,
                                const QString &fullName, int &userIdOut) {
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password, role, full_name) VALUES (?, ?, ?, ?)");
    query.addBindValue(username);
    query.addBindValue(hashedPassword);
    query.addBindValue(role);
    query.addBindValue(fullName);

    if (!query.exec()) {
        Logger::instance()->log("User insert failed: " + query.lastError().text(), Logger::ERROR);
        return false;
    }

    userIdOut = query.lastInsertId().toInt();
    return true;
}

bool RegisterDialog::insertFaceData(int userId, const QByteArray &encoding) {
    QSqlQuery faceQuery;
    faceQuery.prepare("INSERT INTO face_data (user_id, encoding) VALUES (?, ?)");
    faceQuery.addBindValue(userId);
    faceQuery.addBindValue(encoding);
    return faceQuery.exec();
}

void RegisterDialog::on_saveButton_clicked() {
    QString username = ui->usernameLineEdit->text().trimmed();
    QString password = ui->passwordLineEdit->text();
    QString fullName = ui->fullNameLineEdit->text().trimmed();
    QString role = ui->roleComboBox->currentText();

    if (username.isEmpty() || password.isEmpty() || fullName.isEmpty() || capturedFace.empty()) {
        QMessageBox::warning(this, "Missing Info", "Fill all fields and capture face.");
        return;
    }

    if (isUsernameTaken(username)) {
        QMessageBox::warning(this, "Duplicate Username", "This username is already registered.");
        return;
    }

    QProcess process;
    process.setProgram("python");
    process.setArguments({QCoreApplication::applicationDirPath() + "/../../../python_scripts/face_register.py", QCoreApplication::applicationDirPath() + "/../../../python_scripts/captured_frame.jpg"});
    process.start();

    if (!process.waitForFinished()) {
        QMessageBox::warning(this, "Python Error", "Face encoding script failed to run.");
        Logger::instance()->log("Python script failed to finish.", Logger::ERROR);
        return;
    }

    QString output = process.readAllStandardOutput().trimmed();

    if (!output.startsWith("SUCCESS:")) {
        QMessageBox::warning(this, "Face Detection Failed", output);
        Logger::instance()->log("Face registration failed: " + output, Logger::ERROR);
        return;
    }

    QByteArray encoded = QByteArray::fromBase64(output.section(':', 1).toUtf8());
    Logger::instance()->log("Base64 decoded size: " + QString::number(encoded.size()), Logger::INFO);

    int userId;
    QString hashed = QString(QCryptographicHash::hash(password.toUtf8(), QCryptographicHash::Sha256).toHex());

    if (!insertUser(username, hashed, role, fullName, userId)) {
        QMessageBox::warning(this, "Error", "Failed to save user.");
        return;
    }

    if (!insertFaceData(userId, encoded)) {
        QMessageBox::warning(this, "Error", "Failed to save face data.");
        Logger::instance()->log("Face insert failed", Logger::ERROR);
        return;
    }

    QMessageBox::information(this, "Success", "User registered successfully.");
    Logger::instance()->log("User registered: " + username, Logger::INFO);

    ui->usernameLineEdit->clear();
    ui->passwordLineEdit->clear();
    ui->fullNameLineEdit->clear();

    accept();
}

void RegisterDialog::on_cancelButton_clicked() {
    reject();
}

void RegisterDialog::stopCamera() {
    if (cameraThread) {
        cameraThread->stop();
        cameraThread->wait();
        Logger::instance()->log("RegisterDialog camera stopped", Logger::INFO);
        delete cameraThread;
        cameraThread = nullptr;
    }
}
