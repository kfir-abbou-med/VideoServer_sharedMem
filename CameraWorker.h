#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <opencv2/opencv.hpp>
#include <QImage>

class CameraWorker : public QObject {
    Q_OBJECT

public:
    explicit CameraWorker(int cameraIndex, QObject *parent = nullptr);
    ~CameraWorker();

public slots:
    void start();
    void stop();
    void changeBrightness(double factor);
    void changeZoom(double factor);

signals:
    void frameReady(const QImage &image);
    void errorOccurred(const QString &error);

private:
    int cameraIndex;
    bool isRunning;
    cv::VideoCapture capture;
    double brightnessFactor;  
    double zoomFactor;        
};

#endif // CAMERAWORKER_H