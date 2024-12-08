#include <QThread>
#include <QVBoxLayout>
#include <QImage>
#include <QPixmap>
#include <iostream>
#include <opencv2/opencv.hpp>
#include "headers/CameraWindow.h"

using namespace std;

CameraWindow::CameraWindow(int cameraIndex, QWidget *parent)
    : QWidget(parent), worker(nullptr), workerThread(nullptr), brightnessFactor(1.0), zoomFactor(1.0), cameraIndex(cameraIndex){
    setWindowTitle(QString("Camera %1").arg(cameraIndex));
    setFixedSize(200, 200);

    // UI setup
    label = new QLabel(this);
    label->setAlignment(Qt::AlignCenter);

    startButton = new QPushButton("Start", this);
    stopButton = new QPushButton("Stop", this);
    stopButton->setEnabled(false);

    // Create sliders for brightness and zoom
    // brightnessSlider = new QSlider(Qt::Horizontal, this);
    // brightnessSlider->setRange(0, 200);  // 0 to 200 for brightness factor adjustment
    // brightnessSlider->setValue(100); // Default value (normal brightness)

    // zoomSlider = new QSlider(Qt::Horizontal, this);
    // zoomSlider->setRange(100, 200);  // 50% to 150% zoom
    // zoomSlider->setValue(100); // Default value (no zoom)

    QVBoxLayout *layout = new QVBoxLayout(this);
    // layout->addWidget(label);
    layout->addWidget(startButton);
    layout->addWidget(stopButton);
    // Adding video setting buttons
    // layout->addWidget(brightnessSlider);
    // layout->addWidget(zoomSlider);
    setLayout(layout);

    // Worker and thread setup
    // worker = new CameraWorker(cameraIndex, "shared");
    // workerThread = new QThread(this);
    // worker->moveToThread(workerThread);

    // connect(startButton, &QPushButton::clicked, worker, &CameraWorker::start);
    // connect(stopButton, &QPushButton::clicked, worker, &CameraWorker::stop);
    // // connect(worker, &CameraWorker::frameReady, this, &CameraWindow::updateFrame);
    // connect(worker, &CameraWorker::errorOccurred, this, &CameraWindow::handleWorkerError);

    // Connect slider changes to the worker's change methods
    // connect(brightnessSlider, &QSlider::valueChanged, worker, &CameraWorker::changeBrightness);
    // connect(zoomSlider, &QSlider::valueChanged, worker, &CameraWorker::changeZoom);
    // connect(brightnessSlider, &QSlider::valueChanged, this, &CameraWindow::onBrightnessSliderValueChanged);
    // connect(zoomSlider, &QSlider::valueChanged, this, &CameraWindow::onZoomSliderValueChanged);

    connect(workerThread, &QThread::finished, worker, &CameraWorker::deleteLater);
    workerThread->start();
}

// void CameraWindow::updateFrame(const QImage &image) {
//     // Update the label with the new frame
//     label->setPixmap(QPixmap::fromImage(image));
// }
 
// void CameraWindow::onBrightnessSliderValueChanged(int value) {
//     worker->changeBrightness(value / 100.0); // Map the slider value (0-200) to a factor (0.0-2.0)
// }


// void CameraWindow::onZoomSliderValueChanged(int value) {

//     worker->changeZoom(value / 100.0); // Map the slider value (50-150) to a zoom factor (0.5-1.5)

// }

void CameraWindow::handleWorkerError(const QString &error) {
    label->setText(error);

}
CameraWindow::~CameraWindow()
{
    // Stop the worker thread and clean up
    workerThread->quit();
    workerThread->wait();
    delete worker;  // Clean up the worker object
    delete workerThread;  // Clean up the worker thread
}
