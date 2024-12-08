#ifndef VIDEOSETTINGS_H
#define VIDEOSETTINGS_H

#include <string>

class VideoSettings {
public:
    double brightness;
    double zoom;

    // Constructor with default values
    VideoSettings(double brightness = 1, double zoom = 1);

    // Get property value
    double GetPropertyValue(const std::string& propertyName) const;
    
    // Convert to JSON-like string (for debugging or serialization)
    std::string toString() const;
};

#endif // VIDEOSETTINGS_H
