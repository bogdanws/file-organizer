#include "ConfigurationParser.h"
#include <fstream>
#include <algorithm>
#include <cctype>

bool ConfigurationParser::parseFile(const std::string& configFilePath) {
    errors.clear();
    rules.clear();
    
    std::ifstream file(configFilePath);
    if (!file.is_open()) {
        errors.push_back("Could not open configuration file: " + configFilePath);
        return false;
    }
    
    std::string line;
    int lineNumber = 0;
    
    while (std::getline(file, line)) {
        lineNumber++;
        parseLine(line, lineNumber);
        
        // check if this line starts a rule definition
        std::string trimmedLine = trim(line);
        if (trimmedLine == "RULE:") {
            parseRule(file, lineNumber);
        }
    }
    
    // validate required settings
    if (globalConfig.sourceDir.empty()) {
        errors.emplace_back("SOURCE_DIR is required but not specified");
    }
    if (globalConfig.targetBaseDir.empty()) {
        errors.emplace_back("TARGET_BASE_DIR is required but not specified");
    }
    
    return errors.empty();
}

void ConfigurationParser::parseLine(const std::string& line, int lineNumber) {
    const std::string trimmedLine = trim(line);
    
    // skip empty lines and comments
    if (trimmedLine.empty() || trimmedLine[0] == '#') {
        return;
    }
    
    // skip rule definitions (handled separately)
    if (trimmedLine == "RULE:" || trimmedLine == "END_RULE" || 
        trimmedLine == "CONDITIONS:" || trimmedLine == "END_CONDITIONS") {
        return;
    }
    
    // parse global settings
    auto keyValue = splitKeyValue(trimmedLine);
    if (!keyValue.first.empty() && !keyValue.second.empty()) {
        parseGlobalSetting(keyValue.first, keyValue.second);
    }
}

void ConfigurationParser::parseGlobalSetting(const std::string& key, const std::string& value) {
    if (key == "SOURCE_DIR") {
        globalConfig.sourceDir = std::filesystem::path(value);
    } else if (key == "TARGET_BASE_DIR") {
        globalConfig.targetBaseDir = std::filesystem::path(value);
    } else if (key == "DRY_RUN") {
        std::string lowerValue = value;
        std::ranges::transform(lowerValue, lowerValue.begin(), tolower);
        globalConfig.dryRun = (lowerValue == "true" || lowerValue == "yes" || lowerValue == "1");
    } else if (key == "LOG_LEVEL") {
        globalConfig.logLevel = stringToLogLevel(value);
    } else if (key == "LOG_FILE") {
        globalConfig.logFile = value;
    }
}

void ConfigurationParser::parseRule(std::ifstream& file, int& lineNumber) {
    RuleConfig rule;
    std::string line;
    bool inConditions = false;
    
    while (std::getline(file, line)) {
        lineNumber++;
        std::string trimmedLine = trim(line);
        
        if (trimmedLine.empty() || trimmedLine[0] == '#') {
            continue;
        }
        
        if (trimmedLine == "END_RULE") {
            break;
        }
        
        if (trimmedLine == "CONDITIONS:") {
            inConditions = true;
            continue;
        }
        
        if (trimmedLine == "END_CONDITIONS") {
            inConditions = false;
            continue;
        }
        
        auto keyValue = splitKeyValue(trimmedLine);
        if (keyValue.first.empty() || keyValue.second.empty()) {
            continue;
        }
        
        if (inConditions) {
            rule.conditions[keyValue.first] = keyValue.second;
        } else {
            if (keyValue.first == "TARGET_PATH") {
                rule.targetPath = keyValue.second;
            } else if (keyValue.first == "PRIORITY") {
                try {
                    rule.priority = std::stoi(keyValue.second);
                } catch (const std::exception&) {
                    errors.push_back("Invalid priority value at line " + std::to_string(lineNumber) + ": " + keyValue.second);
                    rule.priority = 1000; // default low priority
                }
            } else if (keyValue.first == "APPLIES_TO") {
                rule.appliesTo = keyValue.second;
            }
        }
    }
    
    // validate rule
    if (rule.targetPath.empty()) {
        errors.push_back("Rule missing TARGET_PATH at line " + std::to_string(lineNumber));
        return;
    }
    
    if (rule.appliesTo.empty()) {
        rule.appliesTo = "any"; // default value
    }
    
    // validate APPLIES_TO value
    if (rule.appliesTo != "file" && rule.appliesTo != "folder" && rule.appliesTo != "any") {
        errors.push_back("Invalid APPLIES_TO value: " + rule.appliesTo + " (must be 'file', 'folder', or 'any')");
        return;
    }
    
    rules.push_back(rule);
}

std::string ConfigurationParser::trim(const std::string& str) {
    const auto start = str.find_first_not_of(" \t\r\n");
    if (start == std::string::npos) {
        return "";
    }
    const auto end = str.find_last_not_of(" \t\r\n");
    return str.substr(start, end - start + 1);
}

std::pair<std::string, std::string> ConfigurationParser::splitKeyValue(const std::string& line) {
    const auto colonPos = line.find(':');
    if (colonPos == std::string::npos) {
        return {"", ""};
    }
    
    std::string key = trim(line.substr(0, colonPos));
    std::string value = trim(line.substr(colonPos + 1));
    
    return {key, value};
}

LogLevel ConfigurationParser::stringToLogLevel(const std::string& levelStr) {
    std::string upperLevel = levelStr;
    std::ranges::transform(upperLevel, upperLevel.begin(), toupper);
    
    if (upperLevel == "DEBUG") return LogLevel::DEBUG;
    if (upperLevel == "INFO") return LogLevel::INFO;
    if (upperLevel == "WARNING") return LogLevel::WARNING;
    if (upperLevel == "ERROR") return LogLevel::ERROR;
    
    // default to INFO if unknown
    return LogLevel::INFO;
} 