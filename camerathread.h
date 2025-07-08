// camerathread.h
#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QThread>
#include <QImage>
#include <atomic>
#include <opencv2/opencv.hpp>

class CameraThread : public QThread {
    Q_OBJECT

public:
    CameraThread(QObject *parent = nullptr);
    ~CameraThread();

    void run() override;
    void stop();
    bool isRunning() const;

signals:
    void frameCaptured(const QImage &frame);

private:
    std::atomic<bool> running;
    cv::VideoCapture cap;
};

#endif // CAMERATHREAD_H
