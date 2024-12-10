#ifndef CAMERAWORKER_H
#define CAMERAWORKER_H

#include <QObject>
#include <QThread> 
#include <opencv2/opencv.hpp>
#include <QImage>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
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
    // Message handling methods
    void settingsMgrMessageReceived(const ClientMessage &message); 
    void commandMessageReceived(ClientMessage &message);
    void updateSettingMessageReceived(ClientMessage &message);

    // Camera and processing methods
    void setupCamera();
    void checkCudaAvailability();
    void setupVideoSettings();
    void setupSharedMemory();
    void processFrames();
    
    cv::Mat captureFrame();
    void processFrame(
        const cv::Mat& frame, 
        cv::cuda::GpuMat& gpuFrame, 
        cv::cuda::GpuMat& processedGpuFrame, 
        cv::Mat& processedCpuFrame, 
        cv::cuda::Stream& stream
    );
    
    cv::cuda::GpuMat applyGpuZoom(
        const cv::cuda::GpuMat& inputFrame, 
        const cv::Mat& originalFrame, 
        cv::cuda::Stream& stream
    );
    
    void updateSharedMemory(const cv::Mat& processedCpuFrame);
    void handleStartupError(const std::exception& ex);
    void handleFrameProcessingError(const cv::Exception& e);
    void cleanup();

    // Member variables
    int m_cameraIndex;
    Communication::CommService& m_commService;
    VideoSettingsManager& m_settingsManager;
    char m_sharedMemoryName[32];
    
    // Shared memory related
    boost::interprocess::shared_memory_object shm;
    boost::interprocess::mapped_region region;
    void* sharedMemory;

    std::string currentSrcId;
    std::atomic<bool> isRunning{false};
    std::string sourceKey;
    
    cv::VideoCapture capture;
    double brightnessFactor{1.0};  
    double zoomFactor{1.0};        
};

#endif // CAMERAWORKER_H
// class CameraWorker : public QObject {
//     Q_OBJECT

// public:
//     explicit CameraWorker(int cameraIndex, Communication::CommService& commService, VideoSettingsManager& settingsManager, QObject *parent = nullptr);
//     ~CameraWorker();

// public slots:
//     void start();
//     void stop();

// signals:
//     void frameReady(const QImage &image);
//     void errorOccurred(const QString &error);
//     void fpsUpdated(double fps);

// private:
//     void settingsMgrMessageReceived(const ClientMessage &message); 
//     void commandMessageReceived(ClientMessage &message);
//     void updateSettingMessageReceived(ClientMessage &message);
//     int m_cameraIndex;
//     Communication::CommService& m_commService;
//     VideoSettingsManager& m_settingsManager;
//     char m_sharedMemoryName[32];
//     std::string currentSrcId;
//     bool isRunning;
//     std::string sourceKey;
//     cv::VideoCapture capture;
//     double brightnessFactor;  
//     double zoomFactor;        
// };

// #endif // CAMERAWORKER_H