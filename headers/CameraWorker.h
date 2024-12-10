// #ifndef CAMERAWORKER_H
// #define CAMERAWORKER_H

// #include <QObject>
// #include <QThread> 
// #include <opencv2/opencv.hpp>
// #include <QImage>
// #include <headers/CommService.h>
// #include <headers/VideoSettingsManager.h>
// #include <string>
// #include <tuple>

// class CameraWorker : public QObject {
//     Q_OBJECT

// public:
//     // explicit CameraWorker(int cameraIndex, QObject *parent = nullptr);
//     explicit CameraWorker(int cameraIndex, const std::string& sourceKey, Communication::CommService &commService, VideoSettingsManager& settingsManager, QObject *parent = nullptr);
//     explicit CameraWorker(int cameraIndex, Communication::CommService& commService, VideoSettingsManager& settingsManager, QObject *parent = nullptr);
//     ~CameraWorker();

// public slots:
//     void start();
//     void stop();
//     // void changeBrightness(double factor);
//     // void changeZoom(double factor);

// signals:
//     // void onMessageReceived(const json &message);
//     // void onSettingsMgrMessageReceived(const ClientMessage &message);
//     void frameReady(const QImage &image);
//     void errorOccurred(const QString &error);
//     void fpsUpdated(double fps);

// private:
//     void settingsMgrMessageReceived(const ClientMessage &message); 
//     void onMessageReceived(ClientMessage &message);
//     void onMessageReceived1(ClientMessage &message);
//     VideoSettingsManager& m_settingsManager;
//     Communication::CommService& m_commService;
//     std::string currentSrcId;
//     int m_cameraIndex;
//     bool isRunning;
//     std::string sourceKey;
//     cv::VideoCapture capture;
//     double brightnessFactor;  
//     double zoomFactor;        
//     char m_sharedMemoryName[32];
// };

// #endif // CAMERAWORKER_H