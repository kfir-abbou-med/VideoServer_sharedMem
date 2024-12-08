# Name of the target executable
TARGET = CameraManager

# Specify the template for an application
# TEMPLATE = app

# Add C++ version
CONFIG += c++11

CUDA_DIR = /usr/local/cuda


CUDA_ARCH = sm_50

NVCCFLAGS = --use_fast_math

INCLUDEPATH += \
    /usr/include/opencv4 \
    /usr/include/x86_64-linux-gnu/qt5 \
    /usr/include/x86_64-linux-gnu/qt5/QtWidgets \
    /usr/include/x86_64-linux-gnu/qt5/QtGui \
    /usr/include/x86_64-linux-gnu/qt5/QtCore \
    /usr/include/c++/11 \
    /usr/include/x86_64-linux-gnu/c++/11 \
    /usr/include/c++/11/backward \
    /usr/include/boost \
    /usr/local/cuda/include \
    $$CUDA_DIR/include

CUDA_INC = $$join(INCLUDEPATH,'" -I"','-I"','"')

# Specify CUDA compiler
cuda.input = CUDA_SOURCES
cuda.output = ${QMAKE_FILE_BASE}.o

# CUDA compiler settings
cuda.commands = nvcc -c -arch=$$CUDA_ARCH $$NVCCFLAGS \
                $(INCPATH) ${QMAKE_FILE_IN} -o ${QMAKE_FILE_OUT}

# Dependency type
cuda.dependency_type = TYPE_C

# Add to extra compilers
QMAKE_EXTRA_COMPILERS += cuda

# Ensure CUDA objects are included in compilation
OBJECTS += $(CUDA_SOURCES:.cu=.o)

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
    src/CameraWindow.cpp \
    src/FrameRateTracker.cpp \
    src/CommService.cpp
.cpp

HEADERS += \
    headers/MainWindow.h \
    headers/CameraWorker.h \
    headers/CameraWindow.h \
    headers/FrameRateTracker.h \
    headers/CommService.h \
    headers/MessageUtils.h

# Link against Qt modules
QT += core gui widgets

# Link OpenCV and boost
INCLUDEPATH += /usr/local/include/opencv4

QMAKE_LIBDIR += $$CUDA_DIR/lib/x64

LIBS += -L/usr/local/lib \
    -L/usr/lib/x86_64-linux-gnu \
    -L$$CUDA_DIR/lib64 \
    -lcudart \
    -lcuda \
    -L/usr/local/cuda/lib64 \
    -L/usr/local/lib \
    -lopencv_core \
    -lopencv_highgui \
    -lopencv_imgproc \
    -lopencv_videoio \
    -lopencv_cudaarithm \
    -lopencv_cudawarping \
    -lopencv_cudafilters \
    -lopencv_cudaimgproc \
    -lcudart \
    -lboost_system \
    -lboost_filesystem \
    -lrt  # shared memory library