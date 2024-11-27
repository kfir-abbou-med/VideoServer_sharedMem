#include "headers/CameraWorker.h"
#include "headers/FrameRateTracker.h"
#include <iostream>
#include <boost/interprocess/shared_memory_object.hpp>
#include <boost/interprocess/mapped_region.hpp>
#include <cstring>

using namespace std;
using namespace boost::interprocess;

CameraWorker::CameraWorker(int cameraIndex, QObject *parent)
    : QObject(parent), cameraIndex(cameraIndex), isRunning(false), brightnessFactor(1.0), zoomFactor(1.0) {}

CameraWorker::~CameraWorker()
{
    stop();
}

void CameraWorker::start()
{
    if (isRunning)
        return;
    cout << "Trying to capture video from index: " << cameraIndex << endl;
    capture.open(cameraIndex, cv::CAP_V4L2);
    if (!capture.isOpened())
    {
        emit errorOccurred(QString("Failed to open camera %1").arg(cameraIndex));
        return;
    }
    cout << "Capture is open for index: " << cameraIndex << endl;
    isRunning = true;
    try
    {
        // Shared memory setup
        // string sharedMemoryName = "";
        char camIndexChar[8];
        sprintf(camIndexChar, "%d", cameraIndex);


        char sharedMemoryName[32] = "SharedFrame";
        strcat(sharedMemoryName, camIndexChar);
        
        shared_memory_object shm(open_or_create, sharedMemoryName, read_write);
        shm.truncate(640 * 480 * 3); // Assuming 640x480 resolution, 3 bytes per pixel (RGB)
        mapped_region region(shm, read_write);
        void *sharedMemory = region.get_address();
        FrameRateTracker fpsTracker;
        
        while (isRunning)
        {
           
            cv::Mat frame;
            if (!capture.read(frame) || frame.empty())
            {
                emit errorOccurred(QString("Failed to read frame from camera %1").arg(cameraIndex));
                continue;
            }

            // Apply brightness adjustment
            frame.convertTo(frame, -1, brightnessFactor, 0);

            // Update FPS tracker
            fpsTracker.update();


            // Apply zoom by cropping
            int centerX = frame.cols / 2;
            int centerY = frame.rows / 2;
            int width = frame.cols / zoomFactor;
            int height = frame.rows / zoomFactor;
            cv::Rect zoomRect(centerX - width / 2, centerY - height / 2, width, height);
            cv::Mat zoomedFrame = frame(zoomRect);

            // Resize to the original size to fit the window
            cv::Mat resizedFrame;
            cv::resize(zoomedFrame, resizedFrame, cv::Size(frame.cols, frame.rows));
            std::stringstream fpsText;
            fpsText << "FPS: " << std::fixed << std::setprecision(2) << fpsTracker.getFPS();
            cv::putText(resizedFrame, 
                        fpsText.str(), 
                        cv::Point(10, 30),  // Position of text
                        cv::FONT_HERSHEY_SIMPLEX, 
                        1.0,  // Font scale
                        cv::Scalar(0, 255, 0),  // Green color
                        2);  // Thickness

            // Write frame to shared memory
            if (resizedFrame.total() * resizedFrame.elemSize() <= region.get_size())
            {
                std::memcpy(sharedMemory, resizedFrame.data, resizedFrame.total() * resizedFrame.elemSize());
                // cout << "Frame written to shared memory - Cam index: " << cameraIndex << endl;
            }
            else
            {
                cerr << "Frame size exceeds shared memory region size!" << endl;
            }
        }
    }
    catch (interprocess_exception &ex)
    {
        cerr << "Shared memory error: " << ex.what() << endl;
        emit errorOccurred(QString("Shared memory error: %1").arg(ex.what()));
    }

    capture.release();

    // Cleanup shared memory
    shared_memory_object::remove("SharedFrame");
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
