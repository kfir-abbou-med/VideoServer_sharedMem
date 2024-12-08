#include "headers/VideoSettingsManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>

// Using nlohmann::json
using json = nlohmann::json;

VideoSettings VideoSettingsManager::GetSettings(std::string sourceKey) {
    std::lock_guard<std::mutex> lock(settingsMutex);
    if (settingsMap.find(sourceKey) == settingsMap.end()) {
        settingsMap[sourceKey] = VideoSettings(); // Create default settings
    }
    return settingsMap[sourceKey];
}

void VideoSettingsManager::SetSettings(std::string sourceKey, const VideoSettings& settings) {
    std::lock_guard<std::mutex> lock(settingsMutex);
    settingsMap[sourceKey] = settings;
}

bool VideoSettingsManager::UpdateSetting(const std::string sourceKey, const std::string& settingName, double value) {
    std::lock_guard<std::mutex> lock(settingsMutex);
    auto it = settingsMap.find(sourceKey);
    if (it != settingsMap.end()) {
        if (settingName == "brightness") {
            it->second.brightness = value;
        } else if (settingName == "zoom") {
            it->second.zoom = value;
        } else {
            return false; // Unknown setting
        }
        std::cout << "sourceId: " << sourceKey << " prop: " << settingName << " val: " << value << " updated." << std::endl;
        return true;
    }
    return false; // Source not found
}

void VideoSettingsManager::RemoveSource(std::string sourceKey) {
    std::lock_guard<std::mutex> lock(settingsMutex);
    settingsMap.erase(sourceKey);
}

void VideoSettingsManager::SaveToFile(const std::string& filePath) const {
    std::lock_guard<std::mutex> lock(settingsMutex);
    json j;
    for (const auto& [key, settings] : settingsMap) {
        j[key] = {{"brightness", settings.brightness}, {"zoom", settings.zoom}};
    }

    std::ofstream file(filePath);
    if (file.is_open()) {
        file << j.dump(4); // Pretty-print with 4-space indentation
    } else {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
    }
}

void VideoSettingsManager::LoadFromFile(const std::string& filePath) {
    std::lock_guard<std::mutex> lock(settingsMutex);
    std::ifstream file(filePath);
    if (file.is_open()) {
        json j;
        file >> j;

        settingsMap.clear();
        for (auto& [key, value] : j.items()) {
            settingsMap[key] = VideoSettings(
                value["brightness"].get<int>(),
                value["zoom"].get<int>());
        }
    } else {
        std::cerr << "Failed to open file for reading: " << filePath << std::endl;
    }
}
