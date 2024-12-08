#include <string>
#include <functional>
#include <unordered_map>
#include <mutex>
#include "headers/VideoSettings.h"
#include "headers/VideoSettingsManager.h"
#include <nlohmann/json.hpp>
#include <iostream>
// #include <qlogging.h>

VideoSettingsManager::VideoSettingsManager(const std::string& serverAddress, int serverPort)
    : commServiceRef(serverAddress, serverPort) {
    startListening();
}

VideoSettingsManager::~VideoSettingsManager() {
    commServiceRef.stop();
}

void VideoSettingsManager::RegisterListener(const std::string& sourceKey, std::function<void(const VideoSettings&)> callback) {
    std::lock_guard<std::mutex> lock(settingsMutex); // Ensure thread-safety
    listenersMap[sourceKey] = callback; // Store listener
    std::cout << "Listener registered for source:" << sourceKey << std::endl;
}

void VideoSettingsManager::UnregisterListener(const std::string& sourceKey) {
    std::lock_guard<std::mutex> lock(settingsMutex); // Ensure thread-safety
    listenersMap.erase(sourceKey); // Remove listener
    std::cout << "Listener unregistered for source:" << sourceKey << std::endl;
}


void VideoSettingsManager::startListening() {
    commServiceRef.setMessageReceivedCallback([this](const std::string &message) {
        try {
            auto jsonData = nlohmann::json::parse(message);
            std::string sourceKey = jsonData["sourceId"];
            std::string propertyName = jsonData["propertyName"];
            double propertyValue = jsonData["propertyValue"];

            std::cout << "Received settings update for sourceId: " << sourceKey
                      << ", propertyName: " << propertyName
                      << ", propertyValue: " << propertyValue << std::endl;

            if (UpdateSetting(sourceKey, propertyName, propertyValue)) {
                std::cout << "Updated setting successfully." << std::endl;
            } else {
                std::cerr << "Failed to update setting." << std::endl;
            }
        } catch (const std::exception& e) {
            std::cerr << "Error parsing message: " << e.what() << std::endl;
        }
    });

    commServiceRef.start();
}


VideoSettings VideoSettingsManager::GetSettings(const std::string& sourceKey) {
    std::lock_guard<std::mutex> lock(settingsMutex);
    if (settingsMap.find(sourceKey) == settingsMap.end()) {
        settingsMap[sourceKey] = VideoSettings(); // Create default settings
    }
    return settingsMap[sourceKey];
}

void VideoSettingsManager::SetSettings(const std::string& sourceKey, const VideoSettings& settings) {
    std::lock_guard<std::mutex> lock(settingsMutex);
    settingsMap[sourceKey] = settings;
}

bool VideoSettingsManager::UpdateSetting(const std::string& sourceKey, const std::string& settingName, double value) {
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

        // Notify the listener if it exists
        if (listenersMap.find(sourceKey) != listenersMap.end()) {
            listenersMap[sourceKey](it->second);
        }
        return true;
    }
    return false; // Source not found
}




// #include <string>
// #include <functional>
// #include <unordered_map>
// #include <mutex>
// #include "headers/VideoSettings.h"
// #include "headers/VideoSettingsManager.h"


// void VideoSettingsManager::RegisterListener(const std::string& sourceKey, std::function<void(const VideoSettings&)> callback) {
//     std::lock_guard<std::mutex> lock(settingsMutex);
//     listenersMap[sourceKey] = callback;
// }

// void VideoSettingsManager::UnregisterListener(const std::string& sourceKey) {
//     std::lock_guard<std::mutex> lock(settingsMutex);
//     listenersMap.erase(sourceKey);
// }

// VideoSettings VideoSettingsManager::GetSettings(const std::string& sourceKey) {
//     std::lock_guard<std::mutex> lock(settingsMutex);
//     if (settingsMap.find(sourceKey) == settingsMap.end()) {
//         settingsMap[sourceKey] = VideoSettings(); // Create default settings
//     }
//     return settingsMap[sourceKey];
// }

// bool VideoSettingsManager::UpdateSetting(const std::string& sourceKey, const std::string& settingName, double value) {
//     std::lock_guard<std::mutex> lock(settingsMutex);
//     auto it = settingsMap.find(sourceKey);
//     if (it != settingsMap.end()) {
//         if (settingName == "brightness") {
//             it->second.brightness = value;
//         } else if (settingName == "zoom") {
//             it->second.zoom = value;
//         } else {
//             return false; // Unknown setting
//         }

//         // Notify the listener if it exists
//         if (listenersMap.find(sourceKey) != listenersMap.end()) {
//             listenersMap[sourceKey](it->second);
//         }
//         return true;
//     }
//     return false; // Source not found
// }

// void VideoSettingsManager::RemoveSource(const std::string& sourceKey) {
//     std::lock_guard<std::mutex> lock(settingsMutex);
//     settingsMap.erase(sourceKey);
//     listenersMap.erase(sourceKey); // Remove the listener too
// }














// // #include "headers/VideoSettingsManager.h"
// // #include <nlohmann/json.hpp>
// // #include <fstream>
// // #include <iostream>

// // // Using nlohmann::json
// // using json = nlohmann::json;

// // VideoSettings VideoSettingsManager::GetSettings(std::string sourceKey) {
// //     std::lock_guard<std::mutex> lock(settingsMutex);
// //     if (settingsMap.find(sourceKey) == settingsMap.end()) {
// //         settingsMap[sourceKey] = VideoSettings(); // Create default settings
// //     }
// //     return settingsMap[sourceKey];
// // }

// // void VideoSettingsManager::SetSettings(std::string sourceKey, const VideoSettings& settings) {
// //     std::lock_guard<std::mutex> lock(settingsMutex);
// //     settingsMap[sourceKey] = settings;
// // }

// // bool VideoSettingsManager::UpdateSetting(const std::string sourceKey, const std::string& settingName, double value) {
// //     std::lock_guard<std::mutex> lock(settingsMutex);
// //     auto it = settingsMap.find(sourceKey);
// //     if (it != settingsMap.end()) {
// //         if (settingName == "brightness") {
// //             it->second.brightness = value;
// //         } else if (settingName == "zoom") {
// //             it->second.zoom = value;
// //         } else {
// //             return false; // Unknown setting
// //         }
// //         std::cout << "sourceId: " << sourceKey << " prop: " << settingName << " val: " << value << " updated." << std::endl;
// //         return true;
// //     }
// //     return false; // Source not found
// // }

// // void VideoSettingsManager::RemoveSource(std::string sourceKey) {
// //     std::lock_guard<std::mutex> lock(settingsMutex);
// //     settingsMap.erase(sourceKey);
// // }

// // void VideoSettingsManager::SaveToFile(const std::string& filePath) const {
// //     std::lock_guard<std::mutex> lock(settingsMutex);
// //     json j;
// //     for (const auto& [key, settings] : settingsMap) {
// //         j[key] = {{"brightness", settings.brightness}, {"zoom", settings.zoom}};
// //     }

// //     std::ofstream file(filePath);
// //     if (file.is_open()) {
// //         file << j.dump(4); // Pretty-print with 4-space indentation
// //     } else {
// //         std::cerr << "Failed to open file for writing: " << filePath << std::endl;
// //     }
// // }

// // void VideoSettingsManager::LoadFromFile(const std::string& filePath) {
// //     std::lock_guard<std::mutex> lock(settingsMutex);
// //     std::ifstream file(filePath);
// //     if (file.is_open()) {
// //         json j;
// //         file >> j;

// //         settingsMap.clear();
// //         for (auto& [key, value] : j.items()) {
// //             settingsMap[key] = VideoSettings(
// //                 value["brightness"].get<int>(),
// //                 value["zoom"].get<int>());
// //         }
// //     } else {
// //         std::cerr << "Failed to open file for reading: " << filePath << std::endl;
// //     }
// // }
