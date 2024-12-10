#pragma once

#include <nlohmann/json.hpp>
#include <variant>
#include <string>
#include <optional>
#include <type_traits>
#include <stdexcept>

using json = nlohmann::json;

// Enum for different message types
enum class MessageType {
    UNKNOWN = 0,
    STATUS = 1,
    UPDATE_SETTINGS = 2,
    COMMAND = 3,
    ERROR = 4,
    // Add more message types as needed
};

// Data structures for different message types
struct StatusData {
    std::string message;
    int code;

    // JSON serialization
    json serialize() const {
        return {
            {"message", message},
            {"code", code}
        };
    }

    // JSON deserialization
    static StatusData deserialize(const json& j) {
        return {
            j.value("message", ""),
            j.value("code", 0)
        };
    }
};

struct UpdateSettingsData {
    std::string propertyName;
    std::string propertyValue;

    // JSON serialization
    json serialize() const {
        return {
            {"propertyName", propertyName},
            {"propertyValue", propertyValue}
        };
    }

    // JSON deserialization
    static UpdateSettingsData deserialize(const json& j) {
        return {
            j.value("propertyName", ""),
            j.value("propertyValue", "")
        };
    }
};

struct CommandData {
    std::string command;
    std::vector<std::string> arguments;

    // JSON serialization
    json serialize() const {
        return {
            {"command", command},
            {"arguments", arguments}
        };
    }

    // JSON deserialization
    static CommandData deserialize(const json& j) {
        return {
            j.value("command", ""),
            j.value("arguments", std::vector<std::string>{})
        };
    }
};

struct ErrorData {
    std::string errorMessage;
    int errorCode;

    // JSON serialization
    json serialize() const {
        return {
            {"errorMessage", errorMessage},
            {"errorCode", errorCode}
        };
    }

    // JSON deserialization
    static ErrorData deserialize(const json& j) {
        return {
            j.value("errorMessage", ""),
            j.value("errorCode", 0)
        };
    }
};

class Message {
public:
    // Type alias for possible data types
    using MessageData = std::variant<
        std::monostate,
        StatusData, 
        UpdateSettingsData, 
        CommandData, 
        ErrorData
    >;

    // Constructors
    Message() = default;

    // Constructor with type and data
    template<typename T>
    Message(MessageType type, const T& data) 
        : m_type(type), m_data(data) {}

    // Serialize the entire message to JSON
    json serialize() const {
        json j = {
            {"type", static_cast<int>(m_type)}
        };

        // Serialize data based on type
        std::visit([&j](const auto& data) {
            using T = std::decay_t<decltype(data)>;
            if constexpr (!std::is_same_v<T, std::monostate>) {
                j["data"] = data.serialize();
            }
        }, m_data);

        return j;
    }

    // Deserialize JSON to Message
    static Message deserialize(const json& j) {
        Message msg;
        msg.m_type = static_cast<MessageType>(j.value("type", 0));

        switch (msg.m_type) {
            case MessageType::STATUS:
                msg.m_data = StatusData::deserialize(j["data"]);
                break;
            case MessageType::UPDATE_SETTINGS:
                msg.m_data = UpdateSettingsData::deserialize(j["data"]);
                break;
            case MessageType::COMMAND:
                msg.m_data = CommandData::deserialize(j["data"]);
                break;
            case MessageType::ERROR:
                msg.m_data = ErrorData::deserialize(j["data"]);
                break;
            default:
                msg.m_data = std::monostate{};
        }

        return msg;
    }

    // Get message type
    MessageType getType() const { return m_type; }

    // Get data with type checking
    template<typename T>
    T getData() const {
        try {
            return std::get<T>(m_data);
        } catch (const std::bad_variant_access&) {
            throw std::runtime_error("Invalid data type requested");
        }
    }

    // Check if message contains data
    bool hasData() const {
        return !std::holds_alternative<std::monostate>(m_data);
    }

    // Static factory methods for common message types
    static Message createStatusMessage(const std::string& message, int code = 0) {
        return Message(MessageType::STATUS, StatusData{message, code});
    }

    static Message createUpdateSettingsMessage(const std::string& name, const std::string& value) {
        return Message(MessageType::UPDATE_SETTINGS, UpdateSettingsData{name, value});
    }

    static Message createCommandMessage(const std::string& command, const std::vector<std::string>& args = {}) {
        return Message(MessageType::COMMAND, CommandData{command, args});
    }

    static Message createErrorMessage(const std::string& message, int code = -1) {
        return Message(MessageType::ERROR, ErrorData{message, code});
    }

private:
    MessageType m_type = MessageType::UNKNOWN;
    MessageData m_data;
};