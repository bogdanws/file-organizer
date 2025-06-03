#pragma once

#include <string>
#include <fstream>
#include <mutex>
#include <cstdint>

enum class LogLevel : std::uint8_t {
    DEBUG = 0,
    INFO = 1,
    WARNING = 2,
    ERROR = 3
};

class Logger {
public:
    static Logger& instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    // Initialize the logger with log level and optional file path
    void init(LogLevel level, const std::string& logFilePath = "");
    // Main logging method
    void log(LogLevel level, const std::string& message);
    // Set the current log level
    void setLogLevel(LogLevel level);
    // Convenience methods for different log levels
    void debug(const std::string& message);
    void info(const std::string& message);
    void warning(const std::string& message);
    void error(const std::string& message);
    // Reset logger state (useful for testing)
    void reset();

private:
    Logger() = default;
    ~Logger();

    LogLevel currentLevel = LogLevel::INFO;
    std::string logFilePath;
    std::ofstream logFile;
    std::mutex logMutex;
    bool initialized = false;

    // Helper methods
    static std::string levelToString(LogLevel level);

    static std::string getCurrentTimestamp();
    static void writeToConsole(LogLevel level, const std::string& formattedMessage);
    void writeToFile(const std::string& formattedMessage);
};
