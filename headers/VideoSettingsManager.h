#ifndef VIDEOSETTINGSMANAGER_H
#define VIDEOSETTINGSMANAGER_H

#include "VideoSettings.h"
#include <unordered_map>
#include <string>
#include <mutex>

class VideoSettingsManager {
public:
    // Retrieve settings for a given source, create default if not found
    VideoSettings GetSettings(int sourceKey);

    // Set or update settings for a given source
    void SetSettings(int sourceKey, const VideoSettings& settings);

    // Update a specific setting for a source
    bool UpdateSetting(int sourceKey, const std::string& settingName, double value);

    // Remove settings for a given source
    void RemoveSource(int sourceKey);

    // Save all settings to a JSON file
    void SaveToFile(const std::string& filePath) const;

    // Load all settings from a JSON file
    void LoadFromFile(const std::string& filePath);

private:
    std::unordered_map<int, VideoSettings> settingsMap; // Dictionary for video settings
    mutable std::mutex settingsMutex; // Mutex for thread-safety
};

#endif // VIDEOSETTINGSMANAGER_H
