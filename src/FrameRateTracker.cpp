#include "headers/FrameRateTracker.h"
#include <iostream>
using namespace std;

// Constructor implementation
FrameRateTracker::FrameRateTracker()
{
    frameCount = 0;
    fps = 0.0;
    lastTime = std::chrono::steady_clock::now();
}

// Update method implementation
void FrameRateTracker::update()
{
    frameCount++;
    cout << "[FrameRateTracker::update] Count: " << frameCount << endl;
    if (frameCount >= UPDATE_INTERVAL)
    {
        auto currentTime = std::chrono::steady_clock::now();

        std::chrono::duration<double> duration =
            std::chrono::duration_cast<std::chrono::duration<double>>(currentTime - lastTime);

        fps = frameCount / duration.count();

        // Reset for next interval
        frameCount = 0;
        lastTime = currentTime;
    }
}

// getFPS method implementation
double FrameRateTracker::getFPS() const
{
    return fps;
}