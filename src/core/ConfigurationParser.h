#pragma once

#include <string>
#include <map>
#include <vector>
#include <filesystem>
#include "Logger.h"

struct RuleConfig {
    std::string targetPath;
    int priority;
    std::string appliesTo; // "file", "folder", "any"
    std::map<std::string, std::string> conditions;
};

struct GlobalConfig {
    std::filesystem::path sourceDir;
    std::filesystem::path targetBaseDir;
    bool dryRun = false;
    LogLevel logLevel = LogLevel::INFO;
    std::string logFile;
};

class ConfigurationParser {
public:
    ConfigurationParser() = default;
    ~ConfigurationParser() = default;

    // Parse the configuration file and return global config and rules
    bool parseFile(const std::string& configFilePath);
    
    // Get the parsed global configuration
    const GlobalConfig& getGlobalConfig() const { return globalConfig; }
    
    // Get the parsed rules
    const std::vector<RuleConfig>& getRules() const { return rules; }
    
    // Get any parsing errors
    const std::vector<std::string>& getErrors() const { return errors; }

private:
    GlobalConfig globalConfig;
    std::vector<RuleConfig> rules;
    std::vector<std::string> errors;
    
    // Helper methods
    void parseLine(const std::string& line, int lineNumber);
    void parseGlobalSetting(const std::string& key, const std::string& value);
    void parseRule(std::ifstream& file, int& lineNumber);

    static std::string trim(const std::string& str);
    static std::pair<std::string, std::string> splitKeyValue(const std::string& line);

    static LogLevel stringToLogLevel(const std::string& levelStr);
}; 