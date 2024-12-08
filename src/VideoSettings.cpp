#include "headers/VideoSettings.h"
#include <sstream>
#include <stdexcept>

// Constructor implementation
VideoSettings::VideoSettings(double brightness, double zoom)
    : brightness(brightness), zoom(zoom) {}

// Get property value
double VideoSettings::GetPropertyValue(const std::string& propertyName) const {
    if (propertyName == "brightness")
    {
        return brightness;
    }
    else if(propertyName == "zoom"){
        return zoom;
    } 
    else {
        throw std::invalid_argument("Invalid property name: " + propertyName);
    }
}

// toString implementation
std::string VideoSettings::toString() const {
    std::ostringstream oss;
    oss << "{ brightness: " << brightness << ", zoom: " << zoom << " }";
    return oss.str();
}
