#include "headers/VideoSettingsManager.h"
#include "headers/Message.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <bits/stdc++.h>

// Using nlohmann::json
using json = nlohmann::json;

VideoSettingsManager::VideoSettingsManager(Communication::CommService &commService) : m_commService(commService)
{
    commService.setMessageReceivedCallback([this](Message &message)
                                           { onMessageReceived(message); });
    commService.start();
}

VideoSettingsManager::~VideoSettingsManager()
{
    m_commService.stop();
}

void VideoSettingsManager::RegisterListener(std::string sourceKey, std::function<void(const Message &)> callback)
{
    std::cout << "Register with source id: " << sourceKey << std::endl;
    std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
    m_listenersMap[sourceKey] = callback;
    std::cout << "Register finished for id: " << sourceKey << std::endl;
}

void VideoSettingsManager::UnregisterListener(std::string sourceKey)
{
    std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
    m_listenersMap.erase(sourceKey);
}

void VideoSettingsManager::onMessageReceived(Message &message)
{
    try
    {
        // std::cout << "[VideoSettingsManager::onMessageReceived] " << message << std::endl;
        // // Parse the JSON message

        auto msgType = message.getType();
        std::cout << "Msg type: " << int(msgType) << std::endl;
        // auto jsonData = json::parse(message);
        // auto sourceId = message.at("sourceId");
        // std::string propertyName = message.at("propertyName");
        // double propertyValue = message.at("propertyValue");

        // std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
        // auto it = m_listenersMap.find(sourceId);

        // std::cout << "check for: " << sourceId << " in map with size: " << m_listenersMap.size() << std::endl;
        // for (auto const &x : m_listenersMap)
        // {
        //     std::cout << x.first << std::endl;
        // }

        // if (it != m_listenersMap.end())
        // {
        //     // Fire the listener callback
        //     std::cout << "msg received in settings manager " << message << std::endl;
        //     it->second(message);
        // }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Failed to process message: " << ex.what() << std::endl;
    }
}

// VideoSettings VideoSettingsManager::GetSettings(const std::string sourceKey)
// {
//     std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
//     if (m_settingsMap.find(sourceKey) == m_settingsMap.end())
//     {
//         m_settingsMap[sourceKey] = VideoSettings(); // Create default settings
//     }
//     return m_settingsMap[sourceKey];
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

// Modify other methods similarly to use recursive_mutex
VideoSettings VideoSettingsManager::GetSettings(const std::string sourceKey)
{
    std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);
    if (m_settingsMap.find(sourceKey) == m_settingsMap.end())
    {
        m_settingsMap[sourceKey] = VideoSettings();
    }
    return m_settingsMap[sourceKey];
}

// bool VideoSettingsManager::UpdateSetting(const std::string sourceKey, const std::string &settingName, double value)
// {
//      std::cout << "Thread ID: " << std::this_thread::get_id()
//               << " attempting to update setting" << std::endl;

//     // Check if mutex is already locked by this thread
//     if (m_settingsMutex.try_lock()) {
//         m_settingsMutex.unlock();
//     } else {
//         std::cerr << "Mutex already locked by another thread" << std::endl;
//         return false;
//     }

//     std::lock_guard<std::recursive_mutex> lock(m_settingsMutex);

//     std::cout << "Mutex acquired successfully" << std::endl;

//     auto it = m_settingsMap.find(sourceKey);

//     if (it != m_settingsMap.end())
//     {
//         std::cout << "id: " << sourceKey << "found!" << std::endl;

//         if (settingName == "brightness")
//         {
//             it->second.brightness = value;
//         }
//         else if (settingName == "zoom")
//         {
//             it->second.zoom = value;
//         }
//         else
//         {
//             return false; // Unknown setting
//         }
//         std::cout << "UpdateSetting -> ok" << std::endl;
//         return true;
//     }
//     std::cout << "id: " << sourceKey << "Not found!" << std::endl;

//     return false; // Source not found
// }

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
