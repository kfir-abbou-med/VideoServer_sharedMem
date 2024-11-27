# Name of the target executable
TARGET = CameraManager

# Specify the template for an application
# TEMPLATE = app

# Add C++ version
CONFIG += c++11

INCLUDEPATH += \
    /usr/include/opencv4 \
    /usr/include/x86_64-linux-gnu/qt5 \
    /usr/include/x86_64-linux-gnu/qt5/QtWidgets \
    /usr/include/x86_64-linux-gnu/qt5/QtGui \
    /usr/include/x86_64-linux-gnu/qt5/QtCore \
    /usr/include/c++/11 \
    /usr/include/x86_64-linux-gnu/c++/11 \
    /usr/include/c++/11/backward \
    /usr/include/boost

# Output directories
DESTDIR = $$PWD/bin
OBJECTS_DIR = $$PWD/build/obj
MOC_DIR = $$PWD/build/moc
RCC_DIR = $$PWD/build/rcc
UI_DIR = $$PWD/build/ui

# Sources and headers
SOURCES += \
    src/main.cpp \
    src/Mainwindow.cpp \
    src/CameraWorker.cpp \ 
    src/CameraWindow.cpp

HEADERS += \
    headers/MainWindow.h \
    headers/CameraWorker.h \
    headers/CameraWindow.h

# Link against Qt modules
QT += core gui widgets

# Link OpenCV and boost
INCLUDEPATH += /usr/local/include/opencv4

LIBS += -L/usr/local/lib \
    -L/usr/lib/x86_64-linux-gnu \
    -lopencv_core \
    -lopencv_highgui \
    -lopencv_imgproc \
    -lopencv_videoio \
    -lboost_system \
    -lboost_filesystem \
    -lrt  # shared memory library