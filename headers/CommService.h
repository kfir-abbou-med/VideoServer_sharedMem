#ifndef COMMSERVICE_H
#define COMMSERVICE_H

#include <boost/asio.hpp>
#include <thread>
#include <atomic>
#include <functional>
#include <memory>
#include <string>

namespace Communication
{

    class CommService
    {
    public:
        using MessageReceivedCallback = std::function<void(const std::string &)>;

        // Constructors
        CommService(const std::string &ip, int port);
        CommService();

        // Destructor
        ~CommService();

        // Set the callback to handle received messages
        void setMessageReceivedCallback(MessageReceivedCallback callback);

        // Start the server
        void start();

        // Stop the server
        void stop();

    private:
        // Main server loop
        void run();

        // Accept new client connections
        void acceptConnections();

        // Handle communication with a connected client
        void handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket);

        // Server settings
        std::string m_ip;
        int m_port;

        // Asynchronous IO context and acceptor
        boost::asio::io_context ioContext;
        boost::asio::ip::tcp::acceptor acceptor;

        // Callback for handling received messages
        MessageReceivedCallback m_messageReceivedCallback;

        // Thread for running the IO context
        std::thread workerThread;

        // Atomic flag to indicate if the server is running
        std::atomic<bool> isRunning;
    };

} // namespace Communication

#endif // COMMSERVICE_H
