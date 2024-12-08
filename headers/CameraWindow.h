#ifndef CAMERA_WINDOW_H
#define CAMERA_WINDOW_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>
#include <QSlider>
#include <QThread>
#include <opencv2/opencv.hpp>
#include "CameraWorker.h"

class CameraWindow : public QWidget
{
    Q_OBJECT

public:
    explicit CameraWindow(int cameraIndex, QWidget *parent = nullptr);
    ~CameraWindow();

private slots:
    // void updateFrame(const QImage &image);
    // void onBrightnessSliderValueChanged(int value);
    // void onZoomSliderValueChanged(int value);
    void handleWorkerError(const QString &error);

private:
    QLabel *label;
    QPushButton *startButton;
    QPushButton *stopButton;
    // QSlider *brightnessSlider;
    // QSlider *zoomSlider;
    CameraWorker *worker;
    QThread *workerThread;
    double brightnessFactor;
    double zoomFactor;
    int cameraIndex;
};

#endif // CAMERA_WINDOW_H
