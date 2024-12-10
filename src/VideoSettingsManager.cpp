#include "headers/VideoSettingsManager.h"
#include "headers/Message.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <bits/stdc++.h>

using json = nlohmann::json;

VideoSettingsManager::VideoSettingsManager(){}
// VideoSettingsManager::VideoSettingsManager(Communication::CommService &commService) : m_commService(commService)
// {
//     commService.setMessageReceivedCallback(MessageType::UPDATE_SETTINGS, 
//                                            [this](ClientMessage &message)
//                                            {
//                                                onMessageReceived(message);
//                                            });
//     commService.start();
// }

VideoSettingsManager::~VideoSettingsManager()
{
    // m_commService.stop();
}

// void VideoSettingsManager::RegisterListener(std::string sourceKey, std::function<void(const ClientMessage &)> callback)
// {
//     std::cout << "Register for update settings message with source id: " << sourceKey << std::endl;
//     std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
//     m_listenersMap[sourceKey] = callback;
//     // std::cout << "Register finished for id: " << sourceKey << std::endl;
// }

// void VideoSettingsManager::UnregisterListener(std::string sourceKey)
// {
//     std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
//     m_listenersMap.erase(sourceKey);
// }

// void VideoSettingsManager::onMessageReceived(ClientMessage &message)
// {
//     try
//     {
//         auto msgType = message.getType();
//         std::cout << "Msg type: " << int(msgType) << std::endl;

//         if (msgType == MessageType::UPDATE_SETTINGS)
//         { // update settings
//             auto msgData = message.getData<UpdateSettingsData>();

//             std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
//             auto it = m_listenersMap.find(msgData.sourceId);

//             std::cout << "check for: " << msgData.sourceId << " in map with size: " << m_listenersMap.size() << std::endl;

//             UpdateSetting(msgData.sourceId, msgData.propertyName, stod(msgData.propertyValue));
//             if (it != m_listenersMap.end())
//             {
//                 // Fire the listener callback
//                 // std::cout << "msg received in settings manager " << message << std::endl;
//                 it->second(message);
//             }
//             else{
//                 std::cout << "error: event not fired -> source not found: " << msgData.sourceId << std::endl;
//             }
//         }
//         else{
//             std::cout << "wrong message type cought"<< std::endl;
//         }
//     }
//     catch (const std::exception &ex)
//     {
//         std::cerr << "Failed to process message: " << ex.what() << std::endl;
//     }
// }

void VideoSettingsManager::SetSettings(const std::string sourceKey, const VideoSettings &settings)
{
    std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
    m_settingsMap[sourceKey] = settings;
    std::cout << "setting add by key: " << sourceKey << std::endl;
}

bool VideoSettingsManager::UpdateSetting(const std::string sourceKey, const std::string &settingName, double value)
{
    // Use a lock_guard with recursive_mutex
    std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);

    std::cout << "Updating setting for sourceKey: " << sourceKey
              << ", settingName: " << settingName
              << ", value: " << value << std::endl;

    auto it = m_settingsMap.find(sourceKey);

    if (it == m_settingsMap.end())
    {
        // Create a new entry if not exists
        m_settingsMap[sourceKey] = VideoSettings();
        it = m_settingsMap.find(sourceKey);
    }

    if (settingName == "brightness")
    {
        it->second.brightness = value;
        std::cout << "Brightness updated to: " << value << std::endl;
    }
    else if (settingName == "zoom")
    {
        it->second.zoom = value;
        std::cout << "Zoom updated to: " << value << std::endl;
    }
    else
    {
        std::cout << "Unknown setting: " << settingName << std::endl;
        return false;
    }

    return true;
}

VideoSettings VideoSettingsManager::GetSettings(const std::string sourceKey)
{
    std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
    if (m_settingsMap.find(sourceKey) == m_settingsMap.end())
    {
        m_settingsMap[sourceKey] = VideoSettings();
    }
    return m_settingsMap[sourceKey];
}

void VideoSettingsManager::RemoveSource(std::string sourceKey)
{
    std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
    m_settingsMap.erase(sourceKey);
}

void VideoSettingsManager::SaveToFile(const std::string &filePath) const
{
    std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
    json j;
    for (const auto &[key, settings] : m_settingsMap)
    {
        j[key] = {{"brightness", settings.brightness}, {"zoom", settings.zoom}};
    }

    std::ofstream file(filePath);
    if (file.is_open())
    {
        file << j.dump(4); // Pretty-print with 4-space indentation
    }
    else
    {
        std::cerr << "Failed to open file for writing: " << filePath << std::endl;
    }
}

void VideoSettingsManager::LoadFromFile(const std::string &filePath)
{
    std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
    std::ifstream file(filePath);
    if (file.is_open())
    {
        json j;
        file >> j;

        m_settingsMap.clear();
        for (auto &[key, value] : j.items())
        {
            m_settingsMap[key] = VideoSettings(
                value["brightness"].get<int>(),
                value["zoom"].get<int>());
        }
    }
    else
    {
        std::cerr << "Failed to open file for reading: " << filePath << std::endl;
    }
}
