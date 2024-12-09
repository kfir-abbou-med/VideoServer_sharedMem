#include "headers/MainWindow.h"
#include "headers/CommService.h"
#include "headers/VideoSettingsManager.h"
#include "headers/CameraWorker.h"
// #include "headers/CameraWindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <QThread>
#include <opencv2/opencv.hpp>
#include <iostream>

using namespace std;

MainWindow::MainWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Camera Manager");
    setFixedSize(300, 200);

    QPushButton *detectButton = new QPushButton("Detect Cameras", this);
    detectButton->setFixedSize(200, 50);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(detectButton, 0, Qt::AlignCenter);

    connect(detectButton, &QPushButton::clicked, this, &MainWindow::detectCameras);
}

void MainWindow::detectCameras()
{
    auto commService = new Communication::CommService("127.0.0.1", 8080);
    auto settingsManager = new VideoSettingsManager(*commService);
    int availableCameras[4] = {0, 4};
    int numCameras = 2;

    for (int i = 0; i < numCameras; ++i)
    {
        cv::VideoCapture testCapture;
        int camIndex = availableCameras[i];
        cout << "trying to open cam in index: " << camIndex << endl;

        // Set some properties to ensure compatibility
        testCapture.set(cv::CAP_PROP_FPS, 60);
        testCapture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        testCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
      
        auto cameraWorker = new CameraWorker(camIndex,  *settingsManager);
        cameraWorker->stop();
        auto workerThread = new QThread;

        cameraWorker->moveToThread(workerThread);

        // Start the worker when the thread starts
        connect(workerThread, &QThread::started, cameraWorker, &CameraWorker::start);

        // Cleanup the worker and thread when the thread finishes
        connect(workerThread, &QThread::finished, cameraWorker, &CameraWorker::deleteLater);
        connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

        // Start the thread
        workerThread->start();


    }
}
