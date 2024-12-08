#ifndef VIDEOSETTINGSMANAGER_H
#define VIDEOSETTINGSMANAGER_H

#include "VideoSettings.h"
#include <unordered_map>
#include <string>
#include <mutex>

class VideoSettingsManager {
public:
    // Retrieve settings for a given source, create default if not found
    VideoSettings GetSettings(std::string sourceKey);

    // Set or update settings for a given source
    void SetSettings(std::string sourceKey, const VideoSettings& settings);

    // Update a specific setting for a source
    bool UpdateSetting(std::string sourceKey, const std::string& settingName, double value);

    // Remove settings for a given source
    void RemoveSource(std::string sourceKey);

    // Save all settings to a JSON file
    void SaveToFile(const std::string& filePath) const;

    // Load all settings from a JSON file
    void LoadFromFile(const std::string& filePath);

private:
    std::unordered_map<std::string, VideoSettings> settingsMap; // Dictionary for video settings
    mutable std::mutex settingsMutex; // Mutex for thread-safety
};

#endif // VIDEOSETTINGSMANAGER_H
