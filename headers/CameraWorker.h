#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QThread> 
#include <opencv2/opencv.hpp>
#include <QImage>
#include <headers/CommService.h>
#include <headers/VideoSettingsManager.h>
#include <string>
#include <tuple>

class CameraWorker : public QObject {
    Q_OBJECT

public:
    explicit CameraWorker(int cameraIndex, QObject *parent = nullptr);
    ~CameraWorker();

public slots:
    void start();
    void stop();
    // void changeBrightness(double factor);
    // void changeZoom(double factor);

signals:
    void onMessageReceived(const std::string &message);
    void frameReady(const QImage &image);
    void errorOccurred(const QString &error);
    void fpsUpdated(double fps);

private:
    Communication::CommService commServiceMember;
    VideoSettingsManager settingsManager;
    int cameraIndex;
    bool isRunning;
    cv::VideoCapture capture;
    double brightnessFactor;  
    double zoomFactor;        
};

#endif // CAMERAWORKER_H