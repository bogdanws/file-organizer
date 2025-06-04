#pragma once

#include <string>
#include <chrono>
#include <stdexcept>
#include <regex>
#include <algorithm>
#include <cctype>

// Template function for parsing string values from config file into specific types.
template<typename T>
T parseValue(const std::string& strValue);

// Specialization for basic string type
template<>
inline std::string parseValue<std::string>(const std::string& strValue) {
    return strValue;
}

// Specialization for size values (uintmax_t) - handles KB, MB, GB, etc.
template<>
inline std::uintmax_t parseValue<std::uintmax_t>(const std::string& strValue) {
    if (strValue.empty()) {
        throw std::invalid_argument("Empty size value");
    }
    
    std::string normalized = strValue;
    std::ranges::transform(normalized, normalized.begin(), ::tolower);
    
    // extract numeric part and unit
    std::regex sizeRegex(R"(^(\d+(?:\.\d+)?)\s*(kb|mb|gb|tb|b)?$)");
    std::smatch match;
    
    if (!std::regex_match(normalized, match, sizeRegex)) {
        throw std::invalid_argument("Invalid size format: " + strValue);
    }
    
    double numericValue = std::stod(match[1].str());
    std::string unit = match[2].str();
    
    std::uintmax_t multiplier = 1;
    if (unit == "kb") {
        multiplier = 1024;
    } else if (unit == "mb") {
        multiplier = 1024 * 1024;
    } else if (unit == "gb") {
        multiplier = 1024 * 1024 * 1024;
    } else if (unit == "tb") {
        multiplier = 1024ULL * 1024 * 1024 * 1024;
    }
    // else unit is "b" or empty, multiplier remains 1
    
    return static_cast<std::uintmax_t>(numericValue * multiplier);
}

// Specialization for time duration - handles d (days), M (months), y (years)
template<>
inline std::chrono::system_clock::duration parseValue<std::chrono::system_clock::duration>(const std::string& strValue) {
    if (strValue.empty()) {
        throw std::invalid_argument("Empty duration value");
    }
    
    std::string normalized = strValue;
    std::ranges::transform(normalized, normalized.begin(), ::tolower);
    
    // extract numeric part and unit
    std::regex durationRegex(R"(^(\d+)\s*([dmy])$)");
    std::smatch match;
    
    if (!std::regex_match(normalized, match, durationRegex)) {
        throw std::invalid_argument("Invalid duration format: " + strValue);
    }
    
    int numericValue = std::stoi(match[1].str());
    char unit = match[2].str()[0];
    
    std::chrono::hours duration(0);
    switch (unit) {
        case 'd':
            duration = std::chrono::hours(numericValue * 24);
            break;
        case 'm':
            duration = std::chrono::hours(numericValue * 24 * 30); // approximate month
            break;
        case 'y':
            duration = std::chrono::hours(numericValue * 24 * 365); // approximate year
            break;
        default:
            throw std::invalid_argument("Invalid duration unit: " + std::string(1, unit));
    }
    
    return duration;
}

// Specialization for integers
template<>
inline int parseValue<int>(const std::string& strValue) {
    try {
        return std::stoi(strValue);
    } catch (const std::exception&) {
        throw std::invalid_argument("Invalid integer format: " + strValue);
    }
}

// Specialization for booleans
template<>
inline bool parseValue<bool>(const std::string& strValue) {
    std::string normalized = strValue;
    std::ranges::transform(normalized, normalized.begin(), ::tolower);
    
    if (normalized == "true" || normalized == "1" || normalized == "yes" || normalized == "on") {
        return true;
    } else if (normalized == "false" || normalized == "0" || normalized == "no" || normalized == "off") {
        return false;
    } else {
        throw std::invalid_argument("Invalid boolean format: " + strValue);
    }
}

// External template function helper for safe parsing with default value
template<typename T>
T parseValueSafe(const std::string& strValue, const T& defaultValue) {
    try {
        return parseValue<T>(strValue);
    } catch (const std::exception&) {
        return defaultValue;
    }
} 