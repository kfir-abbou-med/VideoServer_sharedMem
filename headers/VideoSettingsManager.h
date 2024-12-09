#ifndef VIDEOSETTINGSMANAGER_H
#define VIDEOSETTINGSMANAGER_H

#include "VideoSettings.h"
#include "CommService.h"
#include <unordered_map>
#include <string>
#include <mutex>

class VideoSettingsManager {
public:
    VideoSettingsManager(Communication::CommService &commService);
    ~VideoSettingsManager();

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

    void RegisterListener(std::string sourceKey, std::function<void(const std::string&)> callback);
    
    void UnregisterListener(std::string sourceKey);


private:
    std::unordered_map<std::string, VideoSettings> m_settingsMap; // Dictionary for video settings
    std::unordered_map<std::string, std::function<void(const std::string&)>> m_listenersMap;
    // mutable std::mutex m_settingsMutex;
    mutable std::recursive_mutex m_settingsMutex;
    Communication::CommService& m_commService;
    void onMessageReceived(const std::string& message);
};

#endif // VIDEOSETTINGSMANAGER_H
