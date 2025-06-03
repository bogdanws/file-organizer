#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "core/ConfigurationParser.h"

class ConfigurationParserTest : public testing::Test {
protected:
    void SetUp() override {
        testConfigPath = "test_config.txt";
    }
    
    void TearDown() override {
        if (std::filesystem::exists(testConfigPath)) {
            std::filesystem::remove(testConfigPath);
        }
    }
    
    void createTestConfig(const std::string& content) {
        std::ofstream file(testConfigPath);
        file << content;
        file.close();
    }
    
    std::string testConfigPath;
};

TEST_F(ConfigurationParserTest, ParseBasicConfiguration) {
    std::string config = R"(
SOURCE_DIR: /test/source
TARGET_BASE_DIR: /test/target
DRY_RUN: true
LOG_LEVEL: DEBUG
LOG_FILE: test.log
)";
    
    createTestConfig(config);
    
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(testConfigPath));
    
    const auto& globalConfig = parser.getGlobalConfig();
    EXPECT_EQ(globalConfig.sourceDir, "/test/source");
    EXPECT_EQ(globalConfig.targetBaseDir, "/test/target");
    EXPECT_TRUE(globalConfig.dryRun);
    EXPECT_EQ(globalConfig.logLevel, LogLevel::DEBUG);
    EXPECT_EQ(globalConfig.logFile, "test.log");
}

TEST_F(ConfigurationParserTest, ParseSimpleRule) {
    std::string config = R"(
SOURCE_DIR: /test/source
TARGET_BASE_DIR: /test/target

RULE:
  TARGET_PATH: documents/pdf
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE
)";
    
    createTestConfig(config);
    
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(testConfigPath));
    
    const auto& rules = parser.getRules();
    ASSERT_EQ(rules.size(), 1);
    
    const auto& rule = rules[0];
    EXPECT_EQ(rule.targetPath, "documents/pdf");
    EXPECT_EQ(rule.priority, 10);
    EXPECT_EQ(rule.appliesTo, "file");
    EXPECT_EQ(rule.conditions.size(), 1);
    EXPECT_EQ(rule.conditions.at("EXTENSION"), ".pdf");
}

TEST_F(ConfigurationParserTest, ParseMultipleRules) {
    std::string config = R"(
SOURCE_DIR: /test/source
TARGET_BASE_DIR: /test/target

RULE:
  TARGET_PATH: documents/pdf
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: images
  PRIORITY: 20
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .jpg
  END_CONDITIONS
END_RULE
)";
    
    createTestConfig(config);
    
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(testConfigPath));
    
    const auto& rules = parser.getRules();
    ASSERT_EQ(rules.size(), 2);
    
    EXPECT_EQ(rules[0].targetPath, "documents/pdf");
    EXPECT_EQ(rules[0].priority, 10);
    EXPECT_EQ(rules[1].targetPath, "images");
    EXPECT_EQ(rules[1].priority, 20);
}

TEST_F(ConfigurationParserTest, HandleMissingFile) {
    ConfigurationParser parser;
    EXPECT_FALSE(parser.parseFile("nonexistent_file.txt"));
    EXPECT_FALSE(parser.getErrors().empty());
}

TEST_F(ConfigurationParserTest, HandleMissingRequiredFields) {
    std::string config = R"(
# Missing SOURCE_DIR and TARGET_BASE_DIR
DRY_RUN: true
)";
    
    createTestConfig(config);
    
    ConfigurationParser parser;
    EXPECT_FALSE(parser.parseFile(testConfigPath));
    
    const auto& errors = parser.getErrors();
    EXPECT_GE(errors.size(), 2); // Should have errors for missing required fields
}

TEST_F(ConfigurationParserTest, HandleCommentsAndEmptyLines) {
    std::string config = R"(
# This is a comment
SOURCE_DIR: /test/source

# Another comment
TARGET_BASE_DIR: /test/target

# DRY_RUN: false (commented out)
DRY_RUN: true
)";
    
    createTestConfig(config);
    
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(testConfigPath));
    
    const auto& globalConfig = parser.getGlobalConfig();
    EXPECT_EQ(globalConfig.sourceDir, "/test/source");
    EXPECT_EQ(globalConfig.targetBaseDir, "/test/target");
    EXPECT_TRUE(globalConfig.dryRun);
}

TEST_F(ConfigurationParserTest, ParseLogLevels) {
    std::vector<std::pair<std::string, LogLevel>> testCases = {
        {"DEBUG", LogLevel::DEBUG},
        {"INFO", LogLevel::INFO},
        {"WARNING", LogLevel::WARNING},
        {"ERROR", LogLevel::ERROR},
        {"debug", LogLevel::DEBUG}, // Test case insensitivity
        {"INVALID", LogLevel::INFO} // Default fallback
    };
    
    for (const auto& [levelStr, expectedLevel] : testCases) {
        std::string config = "SOURCE_DIR: /test\nTARGET_BASE_DIR: /test\nLOG_LEVEL: " + levelStr;
        createTestConfig(config);
        
        ConfigurationParser parser;
        ASSERT_TRUE(parser.parseFile(testConfigPath));
        
        const auto& globalConfig = parser.getGlobalConfig();
        EXPECT_EQ(globalConfig.logLevel, expectedLevel) << "Failed for level: " << levelStr;
    }
} 