#pragma once

#include <boost/asio.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include <string>
#include <nlohmann/json.hpp>
#include "headers/Message.h" // Include the Message class definition

namespace Communication
{
    class CommService
    {
    public:
        // Callback type for received messages
        using MessageReceivedCallback = std::function<void(ClientMessage &)>;

        // Constructors
        CommService();
        CommService(const std::string &ip, int port);

        // Destructor
        ~CommService();

        // Prevent copying
        CommService(const CommService &) = delete;
        CommService &operator=(const CommService &) = delete;

        // Move semantics
        CommService(CommService &&) noexcept = default;
        CommService &operator=(CommService &&) noexcept = default;

        // Core service methods
        void start();
        void stop();

        // Set callback for received messages
        void setMessageReceivedCallback(MessageType type, const std::string &sourceId, MessageReceivedCallback callback);

        // Send message method
        void sendMessage(const ClientMessage &message, std::shared_ptr<boost::asio::ip::tcp::socket> socket);

        // Getters
        bool isServiceRunning() const { return isRunning; }
        std::string getIpAddress() const { return m_ip; }
        int getPort() const { return m_port; }

    private:
        // Internal methods
        void acceptConnections();
        void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

        // Member variables
        std::string m_ip;
        int m_port;

        // Threading components
        std::thread workerThread;

        std::unordered_map<std::string,
                           std::unordered_map<MessageType, MessageReceivedCallback>>
            m_messageReceivedCallbacks;
        std::atomic<bool> isRunning;

        // Boost.Asio components
        boost::asio::io_context ioContext;
        boost::asio::ip::tcp::acceptor acceptor;
    };
}