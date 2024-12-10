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
    // explicit CameraWorker(int cameraIndex, QObject *parent = nullptr);
    explicit CameraWorker(int cameraIndex, const std::string& sourceKey, VideoSettingsManager& settingsManager, QObject *parent = nullptr);
    explicit CameraWorker(int cameraIndex, VideoSettingsManager& settingsManager, QObject *parent = nullptr);
    // explicit CameraWorker(int cameraIndex, QObject *parent = nullptr);
    ~CameraWorker();

public slots:
    void start();
    void stop();
    // void changeBrightness(double factor);
    // void changeZoom(double factor);

signals:
    void onMessageReceived(const json &message);
    void frameReady(const QImage &image);
    void errorOccurred(const QString &error);
    void fpsUpdated(double fps);

private:
    void handleMessage(const Message &message);
    VideoSettingsManager& m_settingsManager;
    std::string currentSrcId;
    int m_cameraIndex;
    bool isRunning;
    std::string sourceKey;
    cv::VideoCapture capture;
    double brightnessFactor;  
    double zoomFactor;        
};

#endif // CAMERAWORKER_H