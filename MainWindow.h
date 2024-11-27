#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <vector>
#include "CameraWindow.h"    

class MainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

private:
    CameraWindow *cameraWindow;

private slots:
    void detectCameras();
};

#endif // MAINWINDOW_H
