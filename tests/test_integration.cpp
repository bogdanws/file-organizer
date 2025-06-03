#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#include <chrono>
#include <format>
#include "core/ConfigurationParser.h"
#include "core/RuleFactory.h"
#include "core/DirectoryOrganizer.h"
#include "core/Logger.h"

class IntegrationTest : public testing::Test {
protected:
    void SetUp() override {
        // Create unique test directories
        testId = std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        testBaseDir = std::filesystem::temp_directory_path() / ("file_organizer_test_" + testId);
        sourceDir = testBaseDir / "source";
        targetDir = testBaseDir / "target";
        configFile = testBaseDir / "config.txt";
        
        // Create test directories
        std::filesystem::create_directories(sourceDir);
        std::filesystem::create_directories(targetDir);
        
        // Initialize logger to quiet mode for tests
        Logger::instance().init(LogLevel::ERROR);
    }
    
    void TearDown() override {
        // Clean up test directories
        if (std::filesystem::exists(testBaseDir)) {
            std::filesystem::remove_all(testBaseDir);
        }
        
        // Reset logger
        Logger::instance().reset();
    }
    
    void createTestFile(const std::filesystem::path& path, const std::string& content = "test content") {
        std::filesystem::create_directories(path.parent_path());
        std::ofstream file(path);
        file << content;
        file.close();
    }
    
    void createTestDirectory(const std::filesystem::path& path) {
        std::filesystem::create_directories(path);
    }
    
    void createConfigFile(const std::string& config) {
        std::ofstream file(configFile);
        file << config;
        file.close();
    }
    
    bool fileExists(const std::filesystem::path& path) {
        return std::filesystem::exists(path) && std::filesystem::is_regular_file(path);
    }
    
    bool directoryExists(const std::filesystem::path& path) {
        return std::filesystem::exists(path) && std::filesystem::is_directory(path);
    }
    
    size_t countFilesInDirectory(const std::filesystem::path& dir) {
        size_t count = 0;
        if (std::filesystem::exists(dir)) {
            for (const auto& entry : std::filesystem::recursive_directory_iterator(dir)) {
                if (entry.is_regular_file()) {
                    count++;
                }
            }
        }
        return count;
    }
    
    std::string testId;
    std::filesystem::path testBaseDir;
    std::filesystem::path sourceDir;
    std::filesystem::path targetDir;
    std::filesystem::path configFile;
};

TEST_F(IntegrationTest, BasicFileOrganization) {
    // Create test files
    createTestFile(sourceDir / "document1.pdf");
    createTestFile(sourceDir / "document2.txt");
    createTestFile(sourceDir / "image1.jpg");
    createTestFile(sourceDir / "image2.png");
    createTestFile(sourceDir / "random.xyz");
    
    // Create configuration
    std::string config = std::format(R"(
SOURCE_DIR: {}
TARGET_BASE_DIR: {}
DRY_RUN: false
LOG_LEVEL: ERROR

RULE:
  TARGET_PATH: documents/pdf
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: documents/text
  PRIORITY: 20
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .txt
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: images
  PRIORITY: 30
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .jpg
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: images
  PRIORITY: 31
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .png
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: others
  PRIORITY: 1000
  APPLIES_TO: any
  CONDITIONS:
  END_CONDITIONS
END_RULE
)", sourceDir.string(), targetDir.string());
    
    createConfigFile(config);
    
    // Parse configuration and create organizer
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(configFile.string()));
    
    RuleFactory factory;
    auto rules = factory.createRulesFromConfig(parser);
    ASSERT_FALSE(rules.empty());
    
    const auto& globalConfig = parser.getGlobalConfig();
    DirectoryOrganizer organizer(
        globalConfig.sourceDir,
        globalConfig.targetBaseDir,
        std::move(rules),
        globalConfig.dryRun
    );
    
    // Run organization
    organizer.scanAndOrganize();
    
    // Verify results
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 5);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 5);
    EXPECT_EQ(stats.filesSkipped, 0);
    EXPECT_EQ(stats.errors, 0);
    
    // Check file locations
    EXPECT_TRUE(fileExists(targetDir / "documents/pdf/document1.pdf"));
    EXPECT_TRUE(fileExists(targetDir / "documents/text/document2.txt"));
    EXPECT_TRUE(fileExists(targetDir / "images/image1.jpg"));
    EXPECT_TRUE(fileExists(targetDir / "images/image2.png"));
    EXPECT_TRUE(fileExists(targetDir / "others/random.xyz"));
    
    // Source should be empty
    EXPECT_EQ(countFilesInDirectory(sourceDir), 0);
}

TEST_F(IntegrationTest, DryRunMode) {
    // Create test files
    createTestFile(sourceDir / "test1.pdf");
    createTestFile(sourceDir / "test2.txt");
    
    // Create configuration with dry run enabled
    std::string config = std::format(R"(
SOURCE_DIR: {}
TARGET_BASE_DIR: {}
DRY_RUN: true
LOG_LEVEL: ERROR

RULE:
  TARGET_PATH: documents
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: others
  PRIORITY: 1000
  APPLIES_TO: any
  CONDITIONS:
  END_CONDITIONS
END_RULE
)", sourceDir.string(), targetDir.string());
    
    createConfigFile(config);
    
    // Parse configuration and create organizer
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(configFile.string()));
    
    RuleFactory factory;
    auto rules = factory.createRulesFromConfig(parser);
    
    const auto& globalConfig = parser.getGlobalConfig();
    DirectoryOrganizer organizer(
        globalConfig.sourceDir,
        globalConfig.targetBaseDir,
        std::move(rules),
        globalConfig.dryRun
    );
    
    // Run organization
    organizer.scanAndOrganize();
    
    // Verify dry run results
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 2);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 2);
    EXPECT_EQ(stats.errors, 0);
    
    // Files should still be in source (not moved in dry run)
    EXPECT_TRUE(fileExists(sourceDir / "test1.pdf"));
    EXPECT_TRUE(fileExists(sourceDir / "test2.txt"));
    
    // Target should be empty (no actual moves in dry run)
    EXPECT_EQ(countFilesInDirectory(targetDir), 0);
}

TEST_F(IntegrationTest, DirectoryOrganization) {
    // Create test directories with files
    createTestDirectory(sourceDir / "folder1");
    createTestFile(sourceDir / "folder1/file1.txt");
    createTestDirectory(sourceDir / "folder2");
    createTestFile(sourceDir / "folder2/file2.txt");
    createTestDirectory(sourceDir / "empty_folder");
    
    // Create configuration for directories
    std::string config = std::format(R"(
SOURCE_DIR: {}
TARGET_BASE_DIR: {}
DRY_RUN: false
LOG_LEVEL: ERROR

RULE:
  TARGET_PATH: archived_folders
  PRIORITY: 10
  APPLIES_TO: folder
  CONDITIONS:
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: others
  PRIORITY: 1000
  APPLIES_TO: any
  CONDITIONS:
  END_CONDITIONS
END_RULE
)", sourceDir.string(), targetDir.string());
    
    createConfigFile(config);
    
    // Parse and run
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(configFile.string()));
    
    RuleFactory factory;
    auto rules = factory.createRulesFromConfig(parser);
    
    const auto& globalConfig = parser.getGlobalConfig();
    DirectoryOrganizer organizer(
        globalConfig.sourceDir,
        globalConfig.targetBaseDir,
        std::move(rules),
        globalConfig.dryRun
    );
    
    organizer.scanAndOrganize();
    
    // Verify directory moves
    const auto& stats = organizer.getStatistics();
    EXPECT_GT(stats.directoriesProcessed, 0);
    
    // Check that directories were moved
    EXPECT_TRUE(directoryExists(targetDir / "archived_folders/folder1"));
    EXPECT_TRUE(directoryExists(targetDir / "archived_folders/folder2"));
    EXPECT_TRUE(directoryExists(targetDir / "archived_folders/empty_folder"));
    
    // Check that files within directories were moved too
    EXPECT_TRUE(fileExists(targetDir / "archived_folders/folder1/file1.txt"));
    EXPECT_TRUE(fileExists(targetDir / "archived_folders/folder2/file2.txt"));
}

TEST_F(IntegrationTest, FileCollisionHandling) {
    // Create source files
    createTestFile(sourceDir / "document.txt", "source content");
    
    // Create existing file in target
    std::filesystem::create_directories(targetDir / "documents");
    createTestFile(targetDir / "documents/document.txt", "existing content");
    
    // Create configuration
    std::string config = std::format(R"(
SOURCE_DIR: {}
TARGET_BASE_DIR: {}
DRY_RUN: false
LOG_LEVEL: ERROR

RULE:
  TARGET_PATH: documents
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .txt
  END_CONDITIONS
END_RULE
)", sourceDir.string(), targetDir.string());
    
    createConfigFile(config);
    
    // Parse and run
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(configFile.string()));
    
    RuleFactory factory;
    auto rules = factory.createRulesFromConfig(parser);
    
    const auto& globalConfig = parser.getGlobalConfig();
    DirectoryOrganizer organizer(
        globalConfig.sourceDir,
        globalConfig.targetBaseDir,
        std::move(rules),
        globalConfig.dryRun
    );
    
    organizer.scanAndOrganize();
    
    // Verify collision handling
    EXPECT_TRUE(fileExists(targetDir / "documents/document.txt"));  // Original file
    
    // Check for renamed file (should have _001 suffix)
    bool foundRenamed = false;
    for (const auto& entry : std::filesystem::directory_iterator(targetDir / "documents")) {
        if (entry.is_regular_file()) {
            std::string filename = entry.path().filename().string();
            if (filename.find("document_") != std::string::npos && filename.find(".txt") != std::string::npos) {
                foundRenamed = true;
                break;
            }
        }
    }
    EXPECT_TRUE(foundRenamed);
    
    // Source should be empty
    EXPECT_EQ(countFilesInDirectory(sourceDir), 0);
}

TEST_F(IntegrationTest, RulePriorityOrdering) {
    // Create test files
    createTestFile(sourceDir / "test.pdf");
    
    // Create configuration with multiple rules that could match
    std::string config = std::format(R"(
SOURCE_DIR: {}
TARGET_BASE_DIR: {}
DRY_RUN: false
LOG_LEVEL: ERROR

RULE:
  TARGET_PATH: catch_all
  PRIORITY: 1000
  APPLIES_TO: any
  CONDITIONS:
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: specific_pdf
  PRIORITY: 5
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: general_pdf
  PRIORITY: 50
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE
)", sourceDir.string(), targetDir.string());
    
    createConfigFile(config);
    
    // Parse and run
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(configFile.string()));
    
    RuleFactory factory;
    auto rules = factory.createRulesFromConfig(parser);
    
    const auto& globalConfig = parser.getGlobalConfig();
    DirectoryOrganizer organizer(
        globalConfig.sourceDir,
        globalConfig.targetBaseDir,
        std::move(rules),
        globalConfig.dryRun
    );
    
    organizer.scanAndOrganize();
    
    // File should go to highest priority rule (lowest number)
    EXPECT_TRUE(fileExists(targetDir / "specific_pdf/test.pdf"));
    EXPECT_FALSE(fileExists(targetDir / "general_pdf/test.pdf"));
    EXPECT_FALSE(fileExists(targetDir / "catch_all/test.pdf"));
}

TEST_F(IntegrationTest, ErrorHandlingInvalidSource) {
    // Create configuration with non-existent source directory
    std::string config = std::format(R"(
SOURCE_DIR: /nonexistent/directory
TARGET_BASE_DIR: {}
DRY_RUN: false
LOG_LEVEL: ERROR

RULE:
  TARGET_PATH: documents
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE
)", targetDir.string());
    
    createConfigFile(config);
    
    // Parse and run
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(configFile.string()));
    
    RuleFactory factory;
    auto rules = factory.createRulesFromConfig(parser);
    
    const auto& globalConfig = parser.getGlobalConfig();
    DirectoryOrganizer organizer(
        globalConfig.sourceDir,
        globalConfig.targetBaseDir,
        std::move(rules),
        globalConfig.dryRun
    );
    
    organizer.scanAndOrganize();
    
    // Should have errors due to invalid source
    const auto& stats = organizer.getStatistics();
    EXPECT_GT(stats.errors, 0);
    EXPECT_EQ(stats.filesProcessed, 0);
}

TEST_F(IntegrationTest, ComplexDirectoryStructure) {
    // Create files directly in source directory to avoid directory moves
    createTestFile(sourceDir / "report.pdf");
    createTestFile(sourceDir / "data.txt");
    createTestFile(sourceDir / "letter.pdf");
    createTestFile(sourceDir / "photo1.jpg");
    createTestFile(sourceDir / "photo2.png");
    createTestFile(sourceDir / "diagram.jpg");
    createTestFile(sourceDir / "readme.txt");
    createTestFile(sourceDir / "archive.zip");
    
    // Create configuration
    std::string config = std::format(R"(
SOURCE_DIR: {}
TARGET_BASE_DIR: {}
DRY_RUN: false
LOG_LEVEL: ERROR

RULE:
  TARGET_PATH: sorted/documents
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: sorted/text_files
  PRIORITY: 20
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .txt
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: sorted/images
  PRIORITY: 30
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .jpg
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: sorted/images
  PRIORITY: 31
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .png
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: sorted/others
  PRIORITY: 1000
  APPLIES_TO: any
  CONDITIONS:
  END_CONDITIONS
END_RULE
)", sourceDir.string(), targetDir.string());
    
    createConfigFile(config);
    
    // Parse and run
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(configFile.string()));
    
    RuleFactory factory;
    auto rules = factory.createRulesFromConfig(parser);
    
    const auto& globalConfig = parser.getGlobalConfig();
    DirectoryOrganizer organizer(
        globalConfig.sourceDir,
        globalConfig.targetBaseDir,
        std::move(rules),
        globalConfig.dryRun
    );
    
    organizer.scanAndOrganize();
    
    // Verify organization
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 8);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 8);
    EXPECT_EQ(stats.errors, 0);
    
    // Check specific file locations
    EXPECT_TRUE(fileExists(targetDir / "sorted/documents/report.pdf"));
    EXPECT_TRUE(fileExists(targetDir / "sorted/documents/letter.pdf"));
    EXPECT_TRUE(fileExists(targetDir / "sorted/text_files/data.txt"));
    EXPECT_TRUE(fileExists(targetDir / "sorted/text_files/readme.txt"));
    EXPECT_TRUE(fileExists(targetDir / "sorted/images/photo1.jpg"));
    EXPECT_TRUE(fileExists(targetDir / "sorted/images/photo2.png"));
    EXPECT_TRUE(fileExists(targetDir / "sorted/images/diagram.jpg"));
    EXPECT_TRUE(fileExists(targetDir / "sorted/others/archive.zip"));
    
    // Source should be empty except for empty directories
    EXPECT_EQ(countFilesInDirectory(sourceDir), 0);
}

TEST_F(IntegrationTest, EmptySourceDirectory) {
    // Create empty source directory (already created in SetUp)
    
    // Create configuration
    std::string config = std::format(R"(
SOURCE_DIR: {}
TARGET_BASE_DIR: {}
DRY_RUN: false
LOG_LEVEL: ERROR

RULE:
  TARGET_PATH: documents
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE
)", sourceDir.string(), targetDir.string());
    
    createConfigFile(config);
    
    // Parse and run
    ConfigurationParser parser;
    ASSERT_TRUE(parser.parseFile(configFile.string()));
    
    RuleFactory factory;
    auto rules = factory.createRulesFromConfig(parser);
    
    const auto& globalConfig = parser.getGlobalConfig();
    DirectoryOrganizer organizer(
        globalConfig.sourceDir,
        globalConfig.targetBaseDir,
        std::move(rules),
        globalConfig.dryRun
    );
    
    organizer.scanAndOrganize();
    
    // Should complete successfully with no files processed
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 0);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 0);
    EXPECT_EQ(stats.errors, 0);
} 