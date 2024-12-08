#include "headers/MainWindow.h"
#include "headers/CameraWindow.h"
#include <QVBoxLayout>
#include <QPushButton>
#include <QMessageBox>
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
    std::vector<int> availableCameras;
    for (int i = 0; i < 4; ++i) {
        cv::VideoCapture testCapture;
        cout << "trying to open cam in index: " << i << endl;

        // Set some properties to ensure compatibility
        testCapture.set(cv::CAP_PROP_FPS, 60);
        testCapture.set(cv::CAP_PROP_FRAME_WIDTH, 640);
        testCapture.set(cv::CAP_PROP_FRAME_HEIGHT, 480);
        testCapture.open(i, cv::CAP_V4L2);
      
        if (!testCapture.isOpened()) {
            cout << "Failed to open camera at index " << i << endl;
            continue;
        }

        cv::Mat frame;
        if (!testCapture.read(frame)) {
            cout << "Cannot read from camera at index " << i << endl;
            testCapture.release();
            continue;
        }

        if (frame.empty()) {
            cout << "Captured frame is empty at index " << i << endl;
            testCapture.release();
            continue;
        }

        cout << "Successfully opened camera at index: " << i << endl;
        cout << "Frame size: " << frame.cols << "x" << frame.rows << endl;
        
        // availableCameras.push_back(i);
        CameraWindow *cameraWindow = new CameraWindow(i);
        cameraWindow->show();
        testCapture.release();
    }
}