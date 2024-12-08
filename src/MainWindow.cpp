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

    for (int i = 0; i < numCameras; i++)
    {
        // cv::VideoCapture testCapture;
        // cout << "trying to open cam in index: " << availableCameras[i] << endl;

        // // Set some properties to ensure compatibility
        // testCapture.set(cv::CAP_PROP_FPS, 60);
        // testCapture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        // testCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        // testCapture.open(availableCameras[i], cv::CAP_V4L2);

        // if (!testCapture.isOpened()) {
        //     cout << "Failed to open camera at index " << i << endl;
        //     continue;
        // }

        // cv::Mat frame;
        // if (!testCapture.read(frame)) {
        //     cout << "Cannot read from camera at index " << i << endl;
        //     testCapture.release();
        //     continue;
        // }

        // if (frame.empty()) {
        //     cout << "Captured frame is empty at index " << i << endl;
        //     testCapture.release();
        //     continue;
        // }

        // cout << "Successfully opened camera at index: " << i << endl;
        // cout << "Frame size: " << frame.cols << "x" << frame.rows << endl;

        // availableCameras.push_back(i);
        // CameraWindow *cameraWindow = new CameraWindow(i);
        auto cameraWorker = new CameraWorker(availableCameras[i]); // Pass the camera index
        auto workerThread = new QThread(this);

        // Move the worker to the thread
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