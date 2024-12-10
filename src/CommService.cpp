#include "headers/CommService.h"
#include "headers/Message.h"
#include <boost/asio.hpp>
#include <iostream>
#include <thread>
#include <nlohmann/json.hpp>

using json = nlohmann::json;

namespace Communication
{
    CommService::CommService(const std::string &ip, int port)
        : m_ip(ip), m_port(port), m_messageReceivedCallbacks(), isRunning(false), acceptor(ioContext) {}

    CommService::CommService()
        : m_ip(""), m_port(0), m_messageReceivedCallbacks(), isRunning(false), acceptor(ioContext) {}

    CommService::~CommService()
    {
        stop();
    }

    void CommService::setMessageReceivedCallback(MessageType type, const std::string &source, MessageReceivedCallback callback)
    {
        std::cout << "[CommService::setMessageReceivedCallback] "
                  << "source: " << source
                  << ", type: " << static_cast<int>(type)
                  << std::endl;

        m_messageReceivedCallbacks[source][type] = callback;

        // m_messageReceivedCallback[type] = callback;
    }

    void CommService::start()
    {
        if (isRunning.exchange(true))
        {
            std::cerr << "CommService is already running!" << std::endl;
            return;
        }

        try
        {
            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::address::from_string(m_ip), m_port);
            acceptor.open(endpoint.protocol());
            acceptor.set_option(boost::asio::socket_base::reuse_address(true));
            acceptor.bind(endpoint);
            acceptor.listen();

            std::cout << "Server started on " << m_ip << ":" << m_port << std::endl;

            workerThread = std::thread([this]()
                                       { ioContext.run(); });

            acceptConnections();
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error starting server: " << e.what() << std::endl;
            isRunning = false;
        }
    }

    void CommService::stop()
    {
        if (!isRunning.exchange(false))
        {
            return; // Already stopped
        }

        ioContext.stop();

        if (workerThread.joinable())
        {
            workerThread.join();
        }

        std::cout << "Server stopped." << std::endl;
    }

    void CommService::acceptConnections()
    {
        auto socket = std::make_shared<boost::asio::ip::tcp::socket>(ioContext);
        acceptor.async_accept(*socket, [this, socket](const boost::system::error_code &error)
                              {
            if (!error)
            {
                handleClient(socket);
            }
            else
            {
                std::cerr << "Accept error: " << error.message() << std::endl;
            }

            if (isRunning)
            {
                acceptConnections();
            } });
    }

    void CommService::handleClient(std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    {
        auto buffer = std::make_shared<std::vector<char>>(1024);
        socket->async_read_some(boost::asio::buffer(*buffer), [this, socket, buffer](const boost::system::error_code &error, std::size_t bytesTransferred){
            std::cout << "[CommService::handleClient] " << std::endl; 
            if (!error)
            {
                try 
                {
                    // Parse the received JSON message
                    std::string jsonStr(buffer->data(), bytesTransferred);
                    json receivedJson = json::parse(jsonStr);
                    std::cout << "[CommService::handleClient] msg recv" << receivedJson << std::endl;

                    // Deserialize the message
                    ClientMessage message = ClientMessage::deserialize(receivedJson);
                    
                    // Get source and type from the message
                    std::string source = message.getSource();
                    MessageType type = message.getType();

                    std::cout << "Src: " << source << " type: " << int(type) << std::endl;
                     
                    auto sourceIt = m_messageReceivedCallbacks.find(source);
                    if (sourceIt != m_messageReceivedCallbacks.end()) {
                        // Find callback for this message type
                        auto& typeCallbackMap = sourceIt->second;
                        auto typeIt = typeCallbackMap.find(type);
                        
                        if (typeIt != typeCallbackMap.end()) {
                            // Call the specific callback
                            typeIt->second(message);
                            return;
                        }
                    }

                    // Find the callback for this specific message type
                    // auto it = m_messageReceivedCallback.find(message.getType());
                    
                    // // If a callback is registered for this type, invoke it
                    // if (it != m_messageReceivedCallback.end()) {
                    //     it->second(message);
                    //     std::cout << "[CommService::handleClient] fired event: " << std::endl;
                    // }
                    else{
                        std::cout << "Didn't find type: " << int(message.getType()) << std::endl;
                    }

                    // Log the received message type
                    std::cout << "[CommService::handleClient] Received message of type: " 
                              << static_cast<int>(message.getType()) << std::endl;
                }
                catch (const std::exception& e)
                {
                    std::cout << "[CommService::handleClient] Error: " << e.what() << std::endl;
                    std::cerr << "Error parsing message: " << e.what() << std::endl;
                }

                // Keep listening for more messages from the same client
                handleClient(socket);
            }
            else if (error != boost::asio::error::operation_aborted)
            {
                std::cerr << "Client disconnected: " << error.message() << std::endl;
            } });
    }

    // Method to send a message
    void CommService::sendMessage(const ClientMessage &message, std::shared_ptr<boost::asio::ip::tcp::socket> socket)
    {
        try
        {
            // Serialize the message to JSON
            json serializedMessage = message.serialize();
            std::string jsonStr = serializedMessage.dump();

            // Send the serialized message
            boost::asio::async_write(*socket,
                                     boost::asio::buffer(jsonStr),
                                     [](const boost::system::error_code &error, std::size_t /*bytes_transferred*/)
                                     {
                                         if (error)
                                         {
                                             std::cerr << "Error sending message: " << error.message() << std::endl;
                                         }
                                     });
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error serializing message: " << e.what() << std::endl;
        }
    }

} // namespace Communication