#ifndef FRAME_RATE_TRACKER_H
#define FRAME_RATE_TRACKER_H

#include <opencv2/opencv.hpp>
#include <chrono>
#include <iostream>

class FrameRateTracker {
private:
    std::chrono::steady_clock::time_point   lastTime;
    int frameCount;
    double fps;
    const int UPDATE_INTERVAL = 30; // Calculate FPS every 30 frames

public:
    FrameRateTracker();
    void update();
    double getFPS() const; 
};

#endif // FRAME_RATE_TRACKER_H