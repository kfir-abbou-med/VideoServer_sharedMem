#include "headers/MainWindow.h"
#include "headers/CameraWindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
#include <opencv2/opencv.hpp>
#include <iostream>
#include <QThread>

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
    int availableCameras[4] = {0, 4};
    int numCameras = 2; // Number of cameras in the array
    VideoSettingsManager settingsManager("127.0.0.1", 8080);

    for (int i = 0; i < numCameras; i++)
    {
        int camIndex = availableCameras[i];

        auto cameraWorker = new CameraWorker(camIndex, "SharedFrame" + std::to_string(camIndex), settingsManager);
        auto workerThread = new QThread;

        workerThread->setPriority(QThread::NormalPriority);
        cameraWorker->moveToThread(workerThread);
        // Start the worker when the thread starts
        connect(workerThread, &QThread::started, cameraWorker, &CameraWorker::start);

        // Cleanup the worker and thread when the thread finishes
        connect(workerThread, &QThread::finished, cameraWorker, &CameraWorker::deleteLater);
        connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

        // Start the thread
        workerThread->start();

        cout << "Started worker for camera index: " << i << endl;
        // testCapture.release();
    }
}