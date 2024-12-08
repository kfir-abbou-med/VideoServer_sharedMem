#include "headers/CameraWorker.h"
#include "headers/FrameRateTracker.h"
#include "headers/CommService.h"
#include "headers/MessageUtils.h"
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <iostream>
#include <cstring>
#include <nlohmann/json.hpp>
#include <iomanip>
#include <QDebug>
#include <tuple>
#include <QObject>
#include <QThread>

using json = nlohmann::json;
using namespace std;
using namespace boost::interprocess;

CameraWorker::CameraWorker(int cameraIndex, const std::string &sourceKey, VideoSettingsManager &settingsManagerRef, QObject *parent)
    : QObject(parent),
      m_cameraIndex(cameraIndex),
      sourceKey(sourceKey),
      isRunning(false),
      brightnessFactor(1.0),
      zoomFactor(1.0),
      m_settingsManager(settingsManagerRef)
{
}

CameraWorker::CameraWorker(int cameraIndex, VideoSettingsManager &settingsManager, QObject *parent) : QObject(parent), m_cameraIndex(cameraIndex), m_settingsManager(settingsManager)
{
}

CameraWorker::~CameraWorker()
{
    // Unregister listener from VideoSettingsManager
    m_settingsManager.UnregisterListener(sourceKey);
    stop();
}

void CameraWorker::handleMessage(const std::string &message)
{
    cout << "Message received on worker..." << message << endl;
    const auto &[sourceId, propertyName, propertyValue] = deserializeMessage(message);
    cout << "sourceId: " << sourceId << ", propertyName: " << propertyName << ", propertyValue: " << propertyValue << endl;

    try
    {
        std::cout << "calling UpdateSetting " << std::endl;
        bool updated = m_settingsManager.UpdateSetting(sourceId, propertyName, propertyValue);
        if (updated)
        {
            std::cout << "UpdateSetting called successfully" << std::endl;

            VideoSettings srcSettings = m_settingsManager.GetSettings(sourceId);
            zoomFactor = srcSettings.GetPropertyValue("zoom");
            brightnessFactor = srcSettings.GetPropertyValue("brightness");

            cout << "brightnessFactor: " << brightnessFactor << endl;
        }
        else{
            cout << "Update failed... " << endl;
        }
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in UpdateSetting: " << e.what() << std::endl;
    }
}

std::tuple<std::string, std::string, double> deserializeMessage(const std::string &message)
{
    // Parse the JSON message
    auto jsonData = nlohmann::json::parse(message);

    std::string sourceId = jsonData["sourceId"];
    std::string propertyName = jsonData["propertyName"];
    double propertyValue = jsonData["propertyValue"];

    // cout << "deserializeMessage::sourceId: " << sourceId << endl;
    // Return as a tuple
    return std::make_tuple(sourceId, propertyName, propertyValue);
}

void CameraWorker::start()
{
    cout << "CameraWorker::start" << endl;

    if (isRunning)
    {
        cout << "IsRunnning is true" << endl;
        return;
    }
    char sharedMemoryName[32];

    try
    {
        cout << "trying to open cam index: " << m_cameraIndex << endl;

        // Open camera
        capture.open(m_cameraIndex, cv::CAP_V4L2);
        capture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        capture.set(cv::CAP_PROP_BUFFERSIZE, 3);

        // Set specific camera properties
        capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        capture.set(cv::CAP_PROP_FPS, 60);

        if (!capture.isOpened())
        {
            emit errorOccurred(QString("Failed to open camera %1").arg(m_cameraIndex));
            return;
        }

        // Check CUDA availability
        if (cv::cuda::getCudaEnabledDeviceCount() == 0)
        {
            qDebug() << "CUDA is not available!";
            emit errorOccurred("CUDA is not available");
            return;
        }

        // Prepare shared memory
        snprintf(sharedMemoryName, sizeof(sharedMemoryName), "SharedFrame%d", m_cameraIndex);

        // Register listener with VideoSettingsManager
        m_settingsManager.RegisterListener(sharedMemoryName, [this](const std::string &message)
                                           { handleMessage(message); });

        std::cout << "add default video settings: " << std::endl;
        auto setting = VideoSettings();
        m_settingsManager.SetSettings(sharedMemoryName, setting);

        // Remove existing shared memory segment
        boost::interprocess::shared_memory_object::remove(sharedMemoryName);

        // Calculate exact frame size
        const int width = 640;
        const int height = 480;
        const size_t channelSize = width * height;
        const size_t totalSize = channelSize * 3; // RGB channels

        // Create shared memory with precise size
        boost::interprocess::shared_memory_object shm(
            boost::interprocess::open_or_create,
            sharedMemoryName,
            boost::interprocess::read_write);

        // Truncate to exact frame size
        shm.truncate(totalSize);

        boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);
        void *sharedMemory = region.get_address();

        VideoSettings srcSettings = m_settingsManager.GetSettings(currentSrcId);
        zoomFactor = srcSettings.GetPropertyValue("zoom");
        brightnessFactor = srcSettings.GetPropertyValue("brightness");

        isRunning = true;
        FrameRateTracker fpsTracker;

        // Create a CUDA stream for operations
        cv::cuda::Stream stream;

        // Prepare CUDA-related variables outside the loop
        cv::cuda::GpuMat gpuFrame, processedGpuFrame;
        cv::Mat processedCpuFrame;

        while (isRunning)
        {
            cv::Mat frame;
            if (!capture.read(frame) || frame.empty())
            {
                qDebug() << "Failed to read frame from camera" << m_cameraIndex;
                cout << "Failed to read frame from camera" << m_cameraIndex << endl;
                emit errorOccurred(QString("Failed to read frame from camera %1").arg(m_cameraIndex));
                QThread::msleep(25);
                continue;
            }

            try
            {

                // GPU Processing Pipeline
                // 1. Upload frame to GPU
                gpuFrame.upload(frame, stream);
                // cout << "brightness: " << brightnessFactor << endl;
                // 2. GPU Brightness Adjustment
                gpuFrame.convertTo(processedGpuFrame, -1, brightnessFactor, 0, stream);

                // cout << "**** Video Source: " << m_cameraIndex << " ******* brightness: " << brightnessFactor << " ******* zoom: " << zoomFactor << endl;
                // 3. GPU Zoom (using CUDA resize)
                if (zoomFactor != 1.0)
                {
                    int zoomWidth = frame.cols / zoomFactor;
                    int zoomHeight = frame.rows / zoomFactor;

                    // Calculate zoom rectangle
                    int centerX = frame.cols / 2;
                    int centerY = frame.rows / 2;
                    cv::Rect zoomRect(
                        centerX - zoomWidth / 2,
                        centerY - zoomHeight / 2,
                        zoomWidth,
                        zoomHeight);

                    // Crop on GPU
                    cv::cuda::GpuMat gpuZoomedFrame;
                    cv::cuda::resize(processedGpuFrame(zoomRect), gpuZoomedFrame, frame.size(), 0, 0, cv::INTER_LINEAR, stream);
                    processedGpuFrame = gpuZoomedFrame;
                }

                // 4. Download processed frame back to CPU
                processedGpuFrame.download(processedCpuFrame, stream);

                // Synchronize the stream
                stream.waitForCompletion();

                // 5. Update FPS
                fpsTracker.update();

                // 6. Prepare FPS text
                std::stringstream fpsText;
                fpsText << "FPS: " << std::fixed << std::setprecision(2) << fpsTracker.getFPS();
                cout << fpsText.str() << " -> " << sharedMemoryName << endl;

                // 7. Write to shared memory
                size_t frameSize = processedCpuFrame.total() * processedCpuFrame.elemSize();
                if (frameSize <= region.get_size())
                {
                    std::memcpy(sharedMemory, processedCpuFrame.data, frameSize);
                }
                else
                {
                    qDebug() << "Frame size exceeds shared memory size!";
                }
                QThread::msleep(10);
            }
            catch (const cv::Exception &e)
            {
                qDebug() << "OpenCV CUDA Error:" << e.what();
                emit errorOccurred(QString("OpenCV CUDA Error: %1").arg(e.what()));
                break;
            }
        }
    }
    catch (const std::exception &ex)
    {
        qDebug() << "Unexpected error:" << ex.what();
        emit errorOccurred(QString("Unexpected error: %1").arg(ex.what()));
    }
    catch (const runtime_error &e)
    {
        // Handle divide by zero exception
        cout << "Exception: " << e.what() << endl;
    }
    // Cleanup
    capture.release();
    boost::interprocess::shared_memory_object::remove(sharedMemoryName);
    isRunning = false;
}

void CameraWorker::stop()
{
    isRunning = false;
}
