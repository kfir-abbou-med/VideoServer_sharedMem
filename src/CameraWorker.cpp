#include "headers/CameraWorker.h"
#include "headers/FrameRateTracker.h"
#include <opencv2/cudaarithm.hpp>
#include <iostream>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>
#include <opencv2/cudaarithm.hpp>
#include <opencv2/cudawarping.hpp>

#include <iomanip>
#include <QDebug>

#include <QObject>
#include <QThread> // Add this include

using namespace std;
using namespace boost::interprocess;

CameraWorker::CameraWorker(int cameraIndex, QObject *parent)
    : QObject(parent), cameraIndex(cameraIndex), isRunning(false), brightnessFactor(1.0), zoomFactor(1.0) {}

CameraWorker::~CameraWorker()
{
    stop();
}


// void CameraWorker::start()
// {
//     if (isRunning)
//         return;
//     char sharedMemoryName[32];

//     try
//     {
//         // Open camera
//         capture.open(cameraIndex, cv::CAP_V4L2);

//         if (!capture.isOpened())
//         {
//             emit errorOccurred(QString("Failed to open camera %1").arg(cameraIndex));
//             return;
//         }

//         // Check CUDA availability
//         if (cv::cuda::getCudaEnabledDeviceCount() == 0)
//         {
//             qDebug() << "CUDA is not available!";
//             emit errorOccurred("CUDA is not available");
//             return;
//         }

//         // Set specific camera properties
//         capture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
//         capture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

//         // Prepare shared memory
//         snprintf(sharedMemoryName, sizeof(sharedMemoryName), "SharedFrame%d", cameraIndex);

//         // Remove existing shared memory segment
//         boost::interprocess::shared_memory_object::remove(sharedMemoryName);

//         // Calculate exact frame size
//         const int width = 640;
//         const int height = 480;
//         const size_t channelSize = width * height;
//         const size_t totalSize = channelSize * 3; // RGB channels

//         // Create shared memory with precise size
//         boost::interprocess::shared_memory_object shm(
//             boost::interprocess::open_or_create,
//             sharedMemoryName,
//             boost::interprocess::read_write);

//         // Truncate to exact frame size
//         shm.truncate(totalSize);

//         boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);
//         void *sharedMemory = region.get_address();

//         isRunning = true;
//         FrameRateTracker fpsTracker;

//         // Prepare CUDA-related variables outside the loop
//         cv::cuda::GpuMat gpuFrame, processedGpuFrame;
//         cv::Mat processedCpuFrame;

//         while (isRunning)
//         {
//             cv::Mat frame;
//             if (!capture.read(frame) || frame.empty())
//             {
//                 qDebug() << "Failed to read frame from camera" << cameraIndex;
//                 emit errorOccurred(QString("Failed to read frame from camera %1").arg(cameraIndex));
//                 QThread::msleep(25);
//                 continue;
//             }

//             try 
//             {
//                 // GPU Processing Pipeline
//                 // 1. Upload frame to GPU
//                 gpuFrame.upload(frame);

//                 // 2. GPU Brightness Adjustment
//                 gpuFrame.convertTo(processedGpuFrame, -1, brightnessFactor, 0);

//                 // 3. GPU Zoom (using CUDA resize)
//                 if (zoomFactor != 1.0)
//                 {
//                     int zoomWidth = frame.cols / zoomFactor;
//                     int zoomHeight = frame.rows / zoomFactor;
                    
//                     // Calculate zoom rectangle
//                     int centerX = frame.cols / 2;
//                     int centerY = frame.rows / 2;
//                     cv::Rect zoomRect(
//                         centerX - zoomWidth/2, 
//                         centerY - zoomHeight/2, 
//                         zoomWidth, 
//                         zoomHeight
//                     );

//                     // Crop on GPU
//                     cv::cuda::GpuMat gpuZoomedFrame;
//                     cv::cuda::resize(processedGpuFrame(zoomRect), gpuZoomedFrame, frame.size());
//                     processedGpuFrame = gpuZoomedFrame;
//                 }

//                 // 4. Download processed frame back to CPU
//                 processedGpuFrame.download(processedCpuFrame);

//                 // 5. Update FPS
//                 fpsTracker.update();

//                 // 6. Prepare FPS text
//                 std::stringstream fpsText;
//                 fpsText << "FPS: " << std::fixed << std::setprecision(2) << fpsTracker.getFPS();
//                 cout << fpsText.str() << endl;

//                 // 7. Write to shared memory
//                 size_t frameSize = processedCpuFrame.total() * processedCpuFrame.elemSize();
//                 if (frameSize <= region.get_size())
//                 {
//                     std::memcpy(sharedMemory, processedCpuFrame.data, frameSize);
//                 }
//                 else
//                 {
//                     qDebug() << "Frame size exceeds shared memory size!";
//                 }
//             }
//             catch (const cv::Exception &e)
//             {
//                 qDebug() << "OpenCV CUDA Error:" << e.what();
//                 emit errorOccurred(QString("OpenCV CUDA Error: %1").arg(e.what()));
//                 break;
//             }

//             // Small delay to prevent excessive CPU usage
//             cv::cuda::Stream::waitForCompletion(); // Ensure CUDA operations are complete
//         }
//     }
//     catch (const std::exception &ex)
//     {
//         qDebug() << "Unexpected error:" << ex.what();
//         emit errorOccurred(QString("Unexpected error: %1").arg(ex.what()));
//     }

//     // Cleanup
//     capture.release();
//     boost::interprocess::shared_memory_object::remove(sharedMemoryName);
//     isRunning = false;
// }


void CameraWorker::start()
{
    if (isRunning)
        return;
    char sharedMemoryName[32];

    try
    {
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
                // GPU Processing Pipeline
                // 1. Upload frame to GPU
                gpuFrame.upload(frame, stream);

                // 2. GPU Brightness Adjustment
                gpuFrame.convertTo(processedGpuFrame, -1, brightnessFactor, 0, stream);

                // 3. GPU Zoom (using CUDA resize)
                if (zoomFactor != 1.0)
                {
                    int zoomWidth = frame.cols / zoomFactor;
                    int zoomHeight = frame.rows / zoomFactor;
                    
                    // Calculate zoom rectangle
                    int centerX = frame.cols / 2;
                    int centerY = frame.rows / 2;
                    cv::Rect zoomRect(
                        centerX - zoomWidth/2, 
                        centerY - zoomHeight/2, 
                        zoomWidth, 
                        zoomHeight
                    );

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
                cout << fpsText.str() << endl;

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

void CameraWorker::changeBrightness(double factor)
{
    brightnessFactor = factor; // Update the brightness factor
    cout << "BRIGHTNESS -> factor: " << brightnessFactor << endl;
}

void CameraWorker::changeZoom(double factor)
{
    zoomFactor = factor; // Update the zoom factor
    cout << "ZOOM -> factor: " << zoomFactor << endl;
}
