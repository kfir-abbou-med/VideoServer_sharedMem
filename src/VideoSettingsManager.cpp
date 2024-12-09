#include "headers/VideoSettingsManager.h"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <bits/stdc++.h>


// Using nlohmann::json
using json = nlohmann::json;

VideoSettingsManager::VideoSettingsManager(Communication::CommService &commService) : m_commService(commService)
{
    commService.setMessageReceivedCallback([this](const std::string &message)
                                           { onMessageReceived(message); });
    commService.start();
}

VideoSettingsManager::~VideoSettingsManager()
{
    m_commService.stop();
}

// void VideoSettingsManager::RegisterListener(std::string sourceKey, std::function<void(int , const std::string &)> callback)
// {
//     std::lock_guard<std::mutex> lock(m_settingsMutex);
//     m_listenersMap[sourceKey] = callback;
// }

void VideoSettingsManager::RegisterListener(std::string sourceKey, std::function<void(const std::string &)> callback)
{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    m_listenersMap[sourceKey] = callback;
}

// void VideoSettingsManager::UnregisterListener(const std::string &sourceKey)
void VideoSettingsManager::UnregisterListener(std::string sourceKey)
{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    m_listenersMap.erase(sourceKey);
}

void VideoSettingsManager::onMessageReceived(const std::string &message)
{
    try
    {
        // std::cout << message << std::endl;
        // Parse the JSON message
        auto jsonData = json::parse(message);
        auto sourceId = jsonData.at("sourceId");
        std::string propertyName = jsonData.at("propertyName");
        double propertyValue = jsonData.at("propertyValue");

        std::lock_guard<std::mutex> lock(m_settingsMutex);
        auto it = m_listenersMap.find(sourceId);

        std::cout << "check for: " << sourceId << " in map with size: " << m_listenersMap.size() << std::endl;
        for (auto const &x : m_listenersMap)
        {
            std::cout << x.first << std::endl;
        }

        if (it != m_listenersMap.end())
        {
            // Fire the listener callback
            std::cout << "msg received in settings manager " << message << std::endl;
            it->second(message);
        }
    }
    catch (const std::exception &ex)
    {
        std::cerr << "Failed to process message: " << ex.what() << std::endl;
    }
}

VideoSettings VideoSettingsManager::GetSettings(const std::string sourceKey)
{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    if (m_settingsMap.find(sourceKey) == m_settingsMap.end())
    {
        m_settingsMap[sourceKey] = VideoSettings(); // Create default settings
    }
    return m_settingsMap[sourceKey];
}

void VideoSettingsManager::SetSettings(const std::string sourceKey, const VideoSettings &settings)
{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    m_settingsMap[sourceKey] = settings;
}

bool VideoSettingsManager::UpdateSetting(const std::string sourceKey, const std::string &settingName, double value)
{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    auto it = m_settingsMap.find(sourceKey);
    if (it != m_settingsMap.end())
    {
        if (settingName == "brightness")
        {
            it->second.brightness = value;
        }
        else if (settingName == "zoom")
        {
            it->second.zoom = value;
        }
        else
        {
            return false; // Unknown setting
        }

        return true;
    }
    return false; // Source not found
}

void VideoSettingsManager::RemoveSource(std::string sourceKey)
{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
    m_settingsMap.erase(sourceKey);
}

void VideoSettingsManager::SaveToFile(const std::string &filePath) const
{
    std::lock_guard<std::mutex> lock(m_settingsMutex);
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
    std::lock_guard<std::mutex> lock(m_settingsMutex);
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
