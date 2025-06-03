#include <gtest/gtest.h>
#include "core/Logger.h"
#include <filesystem>
#include <fstream>
#include <sstream>

class LoggerTest : public testing::Test {
protected:
    void SetUp() override {
        // Reset logger state before each test
        Logger::instance().reset();

        // Create a temporary log file path
        logFilePath = std::filesystem::temp_directory_path() / "test_log.txt";
        
        // Remove existing log file if it exists
        if (std::filesystem::exists(logFilePath)) {
            std::filesystem::remove(logFilePath);
        }
    }
    
    void TearDown() override {
        // Reset logger and clean up log file
        Logger::instance().reset();
        if (std::filesystem::exists(logFilePath)) {
            std::filesystem::remove(logFilePath);
        }
    }
    
    std::string readLogFile() {
        if (!std::filesystem::exists(logFilePath)) {
            return "";
        }
        
        std::ifstream file(logFilePath);
        std::ostringstream buffer;
        buffer << file.rdbuf();
        return buffer.str();
    }
    
    std::filesystem::path logFilePath;
};

TEST_F(LoggerTest, InitializationWithoutFile) {
    // Test initialization without file logging
    Logger::instance().init(LogLevel::INFO);

    // Should not throw any exceptions
    Logger::instance().info("Test message");
    Logger::instance().debug("Debug message - should not appear");
}

TEST_F(LoggerTest, InitializationWithFile) {
    // Test initialization with file logging
    Logger::instance().init(LogLevel::DEBUG, logFilePath.string());

    Logger::instance().info("Test initialization message");

    // Check that log file was created
    EXPECT_TRUE(std::filesystem::exists(logFilePath));
    
    // Check that initialization message was logged
    std::string logContent = readLogFile();
    EXPECT_TRUE(logContent.find("Logger initialized") != std::string::npos);
    EXPECT_TRUE(logContent.find("Test initialization message") != std::string::npos);
}

TEST_F(LoggerTest, LogLevelFiltering) {
    Logger::instance().init(LogLevel::WARNING, logFilePath.string());

    Logger::instance().debug("Debug message - should not appear");
    Logger::instance().info("Info message - should not appear");
    Logger::instance().warning("Warning message - should appear");
    Logger::instance().error("Error message - should appear");

    std::string logContent = readLogFile();
    
    // Only WARNING and ERROR messages should appear
    EXPECT_TRUE(logContent.find("Debug message") == std::string::npos);
    EXPECT_TRUE(logContent.find("Info message") == std::string::npos);
    EXPECT_TRUE(logContent.find("Warning message") != std::string::npos);
    EXPECT_TRUE(logContent.find("Error message") != std::string::npos);
}

TEST_F(LoggerTest, ConvenienceMethods) {
    Logger::instance().init(LogLevel::DEBUG, logFilePath.string());

    Logger::instance().debug("Debug test");
    Logger::instance().info("Info test");
    Logger::instance().warning("Warning test");
    Logger::instance().error("Error test");

    std::string logContent = readLogFile();
    
    // All messages should appear
    EXPECT_TRUE(logContent.find("[DEBUG] Debug test") != std::string::npos);
    EXPECT_TRUE(logContent.find("[INFO] Info test") != std::string::npos);
    EXPECT_TRUE(logContent.find("[WARNING] Warning test") != std::string::npos);
    EXPECT_TRUE(logContent.find("[ERROR] Error test") != std::string::npos);
}

TEST_F(LoggerTest, LogLevelChange) {
    Logger::instance().init(LogLevel::INFO, logFilePath.string());

    Logger::instance().debug("Debug 1 - should not appear");
    Logger::instance().info("Info 1 - should appear");

    // Change log level to DEBUG
    Logger::instance().setLogLevel(LogLevel::DEBUG);

    Logger::instance().debug("Debug 2 - should appear");
    Logger::instance().info("Info 2 - should appear");

    std::string logContent = readLogFile();
    
    EXPECT_TRUE(logContent.find("Debug 1") == std::string::npos);
    EXPECT_TRUE(logContent.find("Info 1") != std::string::npos);
    EXPECT_TRUE(logContent.find("Debug 2") != std::string::npos);
    EXPECT_TRUE(logContent.find("Info 2") != std::string::npos);
}

TEST_F(LoggerTest, TimestampFormat) {
    Logger::instance().init(LogLevel::INFO, logFilePath.string());

    Logger::instance().info("Timestamp test message");

    std::string logContent = readLogFile();
    
    // Check that timestamp is present in format [YYYY-MM-DD HH:MM:SS]
    // This is a basic check - we're looking for the pattern
    EXPECT_TRUE(logContent.find("[20") != std::string::npos);  // Year starts with 20
    EXPECT_TRUE(logContent.find("] [INFO]") != std::string::npos);  // Proper format
}

