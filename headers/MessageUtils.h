#ifndef MESSAGE_UTILS_H
#define MESSAGE_UTILS_H

#include <string>
#include <tuple>

// Function declaration for deserialization
std::tuple<std::string, double> deserializeMessage(const std::string &message);

#endif // MESSAGE_UTILS_H
