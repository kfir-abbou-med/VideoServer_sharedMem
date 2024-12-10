#include "headers/CameraWorker.h"
#include "headers/FrameRateTracker.h"
#include "headers/CommService.h"
#include "headers/MessageUtils.h"
#include "headers/Message.h"
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
#include <QtConcurrent/QtConcurrent>
#include <QFuture>

using json = nlohmann::json;
using namespace std;
using namespace boost::interprocess;

CameraWorker::CameraWorker(int cameraIndex, Communication::CommService &commService, VideoSettingsManager &settingsManager, QObject *parent) : QObject(parent),
                                                                                                                                               m_cameraIndex(cameraIndex),
                                                                                                                                               m_commService(commService),
                                                                                                                                               m_settingsManager(settingsManager),
                                                                                                                                               m_sharedMemoryName("")
{
    // Prepare shared memory
    snprintf(m_sharedMemoryName, sizeof(m_sharedMemoryName), "SharedFrame%d", cameraIndex);

    boost::interprocess::shared_memory_object::remove(m_sharedMemoryName);
    commService.setMessageReceivedCallback(MessageType::COMMAND, m_sharedMemoryName,
                                           [this](ClientMessage &message)
                                           {
                                               commandMessageReceived(message);
                                           });

    commService.setMessageReceivedCallback(MessageType::UPDATE_SETTINGS, m_sharedMemoryName,
                                           [this](ClientMessage &message)
                                           {
                                               updateSettingMessageReceived(message);
                                           });
}

CameraWorker::~CameraWorker()
{
    stop();
}

void CameraWorker::settingsMgrMessageReceived(const ClientMessage &message)
{
    cout << "Message received on worker..." << endl;
    try
    {
        auto msgData = message.getData<UpdateSettingsData>();
        auto sourceId = message.getSource();

        // can refresh by property or all
        VideoSettings srcSettings = m_settingsManager.GetSettings(sourceId);

        zoomFactor = srcSettings.GetPropertyValue("zoom");
        brightnessFactor = srcSettings.GetPropertyValue("brightness");
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception in UpdateSetting: " << e.what() << std::endl;
    }
}

void CameraWorker::commandMessageReceived(ClientMessage &message)
{
    auto msgType = message.getType();
    std::cout << "[CameraWorker::onMessageReceived] Msg type: " << int(msgType) << std::endl;

    auto msgData = message.getData<CommandData>();
    auto sourceId = message.getSource();

    cout << "source: " << sourceId << ", command: " << msgData.command << ", m_sharedMemoryName: " << m_sharedMemoryName << endl;
    if (sourceId == m_sharedMemoryName)
    {
        cout << "same source" << endl;
        if (msgData.command == "start")
        {
            start();
        }
        else if (msgData.command == "stop")
        {
            stop();
        }
    }
}

void CameraWorker::updateSettingMessageReceived(ClientMessage &message)
{
    cout << "[CameraWorker::onMessageReceived1]" << endl;
    auto msgData = message.getData<UpdateSettingsData>();
    auto sourceId = message.getSource();
    // can refresh by property or all

    m_settingsManager.UpdateSetting(sourceId, msgData.propertyName, stod(msgData.propertyValue));
    auto srcSettings = m_settingsManager.GetSettings(sourceId);

    zoomFactor = srcSettings.GetPropertyValue("zoom");
    brightnessFactor = srcSettings.GetPropertyValue("brightness");
}

void CameraWorker::start()
{
    cout << "[CameraWorker::start] is running: " << isRunning << endl;

    if (isRunning)
    {
        cout << "IsRunnning is true" << endl;
        return;
    }
    QtConcurrent::run([this]()
                      {
                          try
                          {
                              cout << "trying to open cam index: " << m_cameraIndex << endl;

                              // TBD: extract functions and classes

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

                              std::cout << "add default video settings: " << std::endl;
                              auto setting = VideoSettings();
                              m_settingsManager.SetSettings(m_sharedMemoryName, setting);

                              // Remove existing shared memory segment
                              boost::interprocess::shared_memory_object::remove(m_sharedMemoryName);

                              // Calculate exact frame size
                              const int width = 640;
                              const int height = 480;
                              const size_t channelSize = width * height;
                              const size_t totalSize = channelSize * 3; // RGB channels

                              // Create shared memory with precise size
                              boost::interprocess::shared_memory_object shm(
                                  boost::interprocess::open_or_create,
                                  m_sharedMemoryName,
                                  boost::interprocess::read_write);

                              // Truncate to exact frame size
                              shm.truncate(totalSize);

                              boost::interprocess::mapped_region region(shm, boost::interprocess::read_write);
                              void *sharedMemory = region.get_address();

                              VideoSettings srcSettings = m_settingsManager.GetSettings(m_sharedMemoryName);
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
                                      // cout << fpsText.str() << " -> " << sharedMemoryName << endl;

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
                          catch (const runtime_error &e)
                          {
                              // Handle divide by zero exception
                              cout << "Exception: " << e.what() << endl;
                          }
                          catch (const std::exception &ex)
                          {
                              qDebug() << "Unexpected error:" << ex.what();
                              emit errorOccurred(QString("Unexpected error: %1").arg(ex.what()));
                          }

                          // Cleanup
                          capture.release();
                          boost::interprocess::shared_memory_object::remove(m_sharedMemoryName);
                          isRunning = false; });
}

void CameraWorker::stop()
{
    cout << "[CameraWorker::stop]" << endl;
    isRunning = false;
}
