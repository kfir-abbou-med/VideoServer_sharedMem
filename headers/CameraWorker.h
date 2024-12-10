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
    explicit CameraWorker(int cameraIndex, Communication::CommService& commService, VideoSettingsManager& settingsManager, QObject *parent = nullptr);
    ~CameraWorker();

public slots:
    void start();
    void stop();

signals:
    void frameReady(const QImage &image);
    void errorOccurred(const QString &error);
    void fpsUpdated(double fps);

private:
    void settingsMgrMessageReceived(const ClientMessage &message); 
    void commandMessageReceived(ClientMessage &message);
    void updateSettingMessageReceived(ClientMessage &message);
    int m_cameraIndex;
    Communication::CommService& m_commService;
    VideoSettingsManager& m_settingsManager;
    char m_sharedMemoryName[32];
    std::string currentSrcId;
    bool isRunning;
    std::string sourceKey;
    cv::VideoCapture capture;
    double brightnessFactor;  
    double zoomFactor;        
};

#endif // CAMERAWORKER_H