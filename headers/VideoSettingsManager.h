#include <string>
#include <functional>
#include <unordered_map>
#include <mutex>
#include "headers/VideoSettings.h"
#include "headers/CommService.h"

class VideoSettingsManager {
public:
    VideoSettingsManager(const std::string& serverAddress, int serverPort);
    ~VideoSettingsManager();

    VideoSettings GetSettings(const std::string& sourceKey);
    void SetSettings(const std::string& sourceKey, const VideoSettings& settings);
    bool UpdateSetting(const std::string& sourceKey, const std::string& settingName, double value);
    void RemoveSource(const std::string& sourceKey);
    void SaveToFile(const std::string& filePath) const;
    void LoadFromFile(const std::string& filePath);

    // Listener management
    void RegisterListener(const std::string& sourceKey, std::function<void(const VideoSettings&)> callback);
    void UnregisterListener(const std::string& sourceKey);

private:
    void startListening();

    std::unordered_map<std::string, VideoSettings> settingsMap;
    std::unordered_map<std::string, std::function<void(const VideoSettings&)>> listenersMap;
    mutable std::mutex settingsMutex;

    Communication::CommService commServiceRef;
};



// #ifndef VIDEOSETTINGSMANAGER_H
// #define VIDEOSETTINGSMANAGER_H

// #include "VideoSettings.h"
// #include <unordered_map>
// #include <string>
// #include <mutex>

// class VideoSettingsManager {
// public:
//     // Retrieve settings for a given source, create default if not found
//     VideoSettings GetSettings(std::string sourceKey);

//     // Set or update settings for a given source
//     void SetSettings(std::string sourceKey, const VideoSettings& settings);

//     // Update a specific setting for a source
//     bool UpdateSetting(std::string sourceKey, const std::string& settingName, double value);

//     // Remove settings for a given source
//     void RemoveSource(std::string sourceKey);

//     // Save all settings to a JSON file
//     void SaveToFile(const std::string& filePath) const;

//     // Load all settings from a JSON file
//     void LoadFromFile(const std::string& filePath);

// private:
//     std::unordered_map<std::string, VideoSettings> settingsMap; // Dictionary for video settings
//     mutable std::mutex settingsMutex; // Mutex for thread-safety
// };

// #endif // VIDEOSETTINGSMANAGER_H
