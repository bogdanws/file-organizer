#include "core/Logger.h"
#include <iostream>
#include <chrono>
#include <iomanip>
#include <sstream>

Logger& Logger::instance() {
    static Logger instance;
    return instance;
}

Logger::~Logger() {
    if (logFile.is_open()) {
        logFile.close();
    }
}

void Logger::init(const LogLevel level, const std::string& logFilePath) {
    std::lock_guard lock(logMutex);
    currentLevel = level;
    this->logFilePath = logFilePath;
    if (!logFilePath.empty()) {
        logFile.open(logFilePath, std::ios::app);
        if (!logFile.is_open()) {
            std::cerr << "Warning: Could not open log file: " << logFilePath << std::endl;
        }
    }

    initialized = true;

    // log initialization without calling log() to avoid deadlock
    const std::string initMessage = "[" + getCurrentTimestamp() + "] [INFO] Logger initialized with level: " + levelToString(level);
    writeToConsole(LogLevel::INFO, initMessage);
    if (logFile.is_open()) {
        writeToFile(initMessage);
    }
}

void Logger::log(LogLevel level, const std::string& message) {
    if (!initialized) {
        init(LogLevel::INFO);  // initialize with default settings
    }

    if (static_cast<int>(level) < static_cast<int>(currentLevel)) {
        return;  // skip logging if level is below current threshold
    }

    std::lock_guard lock(logMutex);

    const std::string timestamp = getCurrentTimestamp();
    const std::string levelStr = levelToString(level);
    const std::string formattedMessage = "[" + timestamp + "] [" + levelStr + "] " + message;


    // write to file if available
    writeToConsole(level, formattedMessage);
    if (logFile.is_open()) {
        writeToFile(formattedMessage);
    }
}

void Logger::setLogLevel(const LogLevel level) {
    std::lock_guard lock(logMutex);
    currentLevel = level;
}

void Logger::debug(const std::string& message) {
    log(LogLevel::DEBUG, message);
}

void Logger::info(const std::string& message) {
    log(LogLevel::INFO, message);
}

void Logger::warning(const std::string& message) {
    log(LogLevel::WARNING, message);
}

void Logger::error(const std::string& message) {
    log(LogLevel::ERROR, message);
}

void Logger::reset() {
    std::lock_guard lock(logMutex);
    if (logFile.is_open()) {
        logFile.close();
    }
    initialized = false;
    currentLevel = LogLevel::INFO;
    logFilePath = "";
}

std::string Logger::levelToString(const LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO: return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

std::string Logger::getCurrentTimestamp() {
    const auto now = std::chrono::system_clock::now();
    const auto time_t = std::chrono::system_clock::to_time_t(now);
    
    std::ostringstream oss;
    oss << std::put_time(std::localtime(&time_t), "%Y-%m-%d %H:%M:%S");
    return oss.str();
}

void Logger::writeToConsole(const LogLevel level, const std::string& formattedMessage) {
    if (level == LogLevel::ERROR || level == LogLevel::WARNING) {
        std::cerr << formattedMessage << std::endl;
    } else {
        std::cout << formattedMessage << std::endl;
    }
}

void Logger::writeToFile(const std::string& formattedMessage) {
    if (logFile.is_open()) {
        logFile << formattedMessage << std::endl;
        logFile.flush();  // ensure immediate write
    }
}
