#include "headers/CameraWorker.h"
#include "headers/FrameRateTracker.h"
#include "headers/CommService.h"
#include "headers/MessageUtils.h"
#include <opencv2/cudaarithm.hpp>
#include <iostream>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>
#include <nlohmann/json.hpp>
#include <iomanip>
#include <QDebug>
#include <tuple>
#include <QObject>
#include <QThread>

using json = nlohmann::json;

using namespace std;
using namespace boost::interprocess;

CameraWorker::CameraWorker(int cameraIndex, QObject *parent)
    : QObject(parent), cameraIndex(cameraIndex), isRunning(false), brightnessFactor(1.0), zoomFactor(1.0), commServiceMember("127.0.0.1", 9500)
{
}

CameraWorker::~CameraWorker()
{
    stop();
}

std::tuple<std::string, std::string, double> deserializeMessage(const std::string &message)
{
    // Parse the JSON message
    auto jsonData = nlohmann::json::parse(message);

    std::string sourceId = jsonData["sourceId"];
    std::string propertyName = jsonData["propertyName"];
    double propertyValue = jsonData["propertyValue"];

    cout << "deserializeMessage::sourceId: " << sourceId << endl;
    // Return as a tuple
    return std::make_tuple(sourceId, propertyName, propertyValue);
}

void CameraWorker::start()
{
    cout << "CameraWorker::start" << endl;
    if (isRunning)
        return;
    char sharedMemoryName[32];

    try
    {
        // Open socket
        Communication::CommService server("127.0.0.1", 8080);

        server.setMessageReceivedCallback([this](const std::string &message)
                                          {
                                              const auto &[sourceId, propertyName, propertyValue] = deserializeMessage(message);
                                              cout << "sourceId: " << sourceId << " received msg: " << message << "got value from message: " << propertyValue << endl;
                                              currentSrcId = sourceId;
                                              settingsManager.UpdateSetting(currentSrcId, propertyName, propertyValue);
                                          });

        server.start();

        cout << "trying to open cam index: " << cameraIndex << endl;

        // Open camera
        capture.open(cameraIndex, cv::CAP_V4L2);
        capture.set(cv::CAP_PROP_FOURCC, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'));
        capture.set(cv::CAP_PROP_BUFFERSIZE, 3);

        // Set specific camera properties
        capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        capture.set(cv::CAP_PROP_FPS, 60);

        if (!capture.isOpened())
        {
            emit errorOccurred(QString("Failed to open camera %1").arg(cameraIndex));
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
        snprintf(sharedMemoryName, sizeof(sharedMemoryName), "SharedFrame%d", cameraIndex);

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
                qDebug() << "Failed to read frame from camera" << cameraIndex;
                cout << "Failed to read frame from camera" << cameraIndex << endl;
                emit errorOccurred(QString("Failed to read frame from camera %1").arg(cameraIndex));
                QThread::msleep(25);
                continue;
            }

            try
            {
                VideoSettings srcSettings = settingsManager.GetSettings(currentSrcId);
                double zoomFactor = srcSettings.GetPropertyValue("zoom");
                double brightnessFactor = srcSettings.GetPropertyValue("brightness");

                // GPU Processing Pipeline
                // 1. Upload frame to GPU
                gpuFrame.upload(frame, stream);
                // cout << "brightness: " << brightnessFactor << endl;
                // 2. GPU Brightness Adjustment
                gpuFrame.convertTo(processedGpuFrame, -1, brightnessFactor, 0, stream);


                // cout << "**** Video Source: " << cameraIndex << " ******* brightness: " << brightnessFactor << " ******* zoom: " << zoomFactor << endl;
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
                // cout << fpsText.str() << endl;

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

    // Cleanup
    capture.release();
    boost::interprocess::shared_memory_object::remove(sharedMemoryName);
    isRunning = false;
}

void CameraWorker::stop()
{
    isRunning = false;
}
 