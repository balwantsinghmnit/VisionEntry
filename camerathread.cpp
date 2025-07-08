// camerathread.cpp
#include "camerathread.h"
#include <QImage>
#include <QDebug>
#include <QFile>
#include "logger.h"


CameraThread::CameraThread(QObject *parent)
    : QThread(parent), running(false) {
}

CameraThread::~CameraThread() {
    stop();
    wait();
}

void CameraThread::stop() {
    running.store(false);
}

void CameraThread::run() {
    cap.open(0);
    if (!cap.isOpened()) {
        Logger::instance()->log("Failed to open webcam in CameraThread", Logger::ERROR);
        return;
    }

    running.store(true);

    cv::Mat frame;
    while (running.load()) {
        cap.read(frame);
        if (frame.empty()) continue;

        QImage img(frame.data, frame.cols, frame.rows, frame.step, QImage::Format_BGR888);
        emit frameCaptured(img.copy());

        msleep(30);
    }

    cap.release();
}

bool CameraThread::isRunning() const {
    return running.load();
}
