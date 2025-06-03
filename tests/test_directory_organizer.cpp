#include <gtest/gtest.h>
#include <filesystem>
#include <fstream>
#include <memory>
#include <chrono>
#include "core/DirectoryOrganizer.h"
#include "core/Logger.h"
#include "rules/ConfigurableRule.h"
#include "conditions/ExtensionCondition.h"

class DirectoryOrganizerTest : public testing::Test {
protected:
    void SetUp() override {
        // Create unique test directories
        testId = std::to_string(std::chrono::high_resolution_clock::now().time_since_epoch().count());
        testBaseDir = std::filesystem::temp_directory_path() / ("dir_organizer_test_" + testId);
        sourceDir = testBaseDir / "source";
        targetDir = testBaseDir / "target";
        
        // Create test directories
        std::filesystem::create_directories(sourceDir);
        std::filesystem::create_directories(targetDir);
        
        // Initialize logger to quiet mode for tests
        Logger::instance().init(LogLevel::ERROR);
        
        // Create test rules
        createTestRules();
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
    
    void createTestRules() {
        rules.clear();
        
        // PDF rule
        auto pdfRule = std::make_unique<ConfigurableRule>("documents/pdf", 10);
        pdfRule->addCondition(std::make_unique<ExtensionCondition>(".pdf"));
        rules.push_back(std::move(pdfRule));
        
        // TXT rule
        auto txtRule = std::make_unique<ConfigurableRule>("documents/text", 20);
        txtRule->addCondition(std::make_unique<ExtensionCondition>(".txt"));
        rules.push_back(std::move(txtRule));
        
        // Catch-all rule
        auto catchAllRule = std::make_unique<ConfigurableRule>("others", 1000);
        rules.push_back(std::move(catchAllRule));
    }
    
    std::vector<std::unique_ptr<ISortingRule>> copyRules() {
        std::vector<std::unique_ptr<ISortingRule>> rulesCopy;
        
        // PDF rule
        auto pdfRule = std::make_unique<ConfigurableRule>("documents/pdf", 10);
        pdfRule->addCondition(std::make_unique<ExtensionCondition>(".pdf"));
        rulesCopy.push_back(std::move(pdfRule));
        
        // TXT rule
        auto txtRule = std::make_unique<ConfigurableRule>("documents/text", 20);
        txtRule->addCondition(std::make_unique<ExtensionCondition>(".txt"));
        rulesCopy.push_back(std::move(txtRule));
        
        // Catch-all rule
        auto catchAllRule = std::make_unique<ConfigurableRule>("others", 1000);
        rulesCopy.push_back(std::move(catchAllRule));
        
        return rulesCopy;
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
    std::vector<std::unique_ptr<ISortingRule>> rules;
};

TEST_F(DirectoryOrganizerTest, ConstructorInitialization) {
    DirectoryOrganizer organizer(sourceDir, targetDir, copyRules(), false);
    
    // Test initial statistics
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 0);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 0);
    EXPECT_EQ(stats.filesSkipped, 0);
    EXPECT_EQ(stats.directoriesProcessed, 0);
    EXPECT_EQ(stats.directoriesMovedOrWouldMove, 0);
    EXPECT_EQ(stats.directoriesSkipped, 0);
    EXPECT_EQ(stats.errors, 0);
}

TEST_F(DirectoryOrganizerTest, DryRunMode) {
    // Create test files
    createTestFile(sourceDir / "test.pdf");
    createTestFile(sourceDir / "test.txt");
    
    DirectoryOrganizer organizer(sourceDir, targetDir, copyRules(), true); // Dry run enabled
    organizer.scanAndOrganize();
    
    // Verify statistics
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 2);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 2);
    EXPECT_EQ(stats.filesSkipped, 0);
    EXPECT_EQ(stats.errors, 0);
    
    // Files should still exist in source (dry run)
    EXPECT_TRUE(std::filesystem::exists(sourceDir / "test.pdf"));
    EXPECT_TRUE(std::filesystem::exists(sourceDir / "test.txt"));
    
    // Target should be empty
    EXPECT_EQ(countFilesInDirectory(targetDir), 0);
}

TEST_F(DirectoryOrganizerTest, ActualFileMove) {
    // Create test files
    createTestFile(sourceDir / "document.pdf");
    createTestFile(sourceDir / "readme.txt");
    createTestFile(sourceDir / "unknown.xyz");
    
    DirectoryOrganizer organizer(sourceDir, targetDir, copyRules(), false); // Actual move
    organizer.scanAndOrganize();
    
    // Verify statistics
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 3);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 3);
    EXPECT_EQ(stats.filesSkipped, 0);
    EXPECT_EQ(stats.errors, 0);
    
    // Verify file locations
    EXPECT_TRUE(std::filesystem::exists(targetDir / "documents/pdf/document.pdf"));
    EXPECT_TRUE(std::filesystem::exists(targetDir / "documents/text/readme.txt"));
    EXPECT_TRUE(std::filesystem::exists(targetDir / "others/unknown.xyz"));
    
    // Source should be empty
    EXPECT_EQ(countFilesInDirectory(sourceDir), 0);
}

TEST_F(DirectoryOrganizerTest, StatisticsReset) {
    // Create a test file and run organization
    createTestFile(sourceDir / "test.pdf");
    
    DirectoryOrganizer organizer(sourceDir, targetDir, copyRules(), false);
    organizer.scanAndOrganize();
    
    // Verify initial stats
    auto stats = organizer.getStatistics();
    EXPECT_GT(stats.filesProcessed, 0);
    
    // Reset statistics
    organizer.resetStatistics();
    stats = organizer.getStatistics();
    
    // All stats should be zero
    EXPECT_EQ(stats.filesProcessed, 0);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 0);
    EXPECT_EQ(stats.filesSkipped, 0);
    EXPECT_EQ(stats.directoriesProcessed, 0);
    EXPECT_EQ(stats.directoriesMovedOrWouldMove, 0);
    EXPECT_EQ(stats.directoriesSkipped, 0);
    EXPECT_EQ(stats.errors, 0);
}

TEST_F(DirectoryOrganizerTest, InvalidSourceDirectory) {
    std::filesystem::path nonExistentSource = testBaseDir / "nonexistent";
    
    DirectoryOrganizer organizer(nonExistentSource, targetDir, copyRules(), false);
    organizer.scanAndOrganize();
    
    // Should have errors
    const auto& stats = organizer.getStatistics();
    EXPECT_GT(stats.errors, 0);
    EXPECT_EQ(stats.filesProcessed, 0);
}

TEST_F(DirectoryOrganizerTest, EmptySourceDirectory) {
    // Source directory exists but is empty
    DirectoryOrganizer organizer(sourceDir, targetDir, copyRules(), false);
    organizer.scanAndOrganize();
    
    // Should complete successfully with no files processed
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 0);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 0);
    EXPECT_EQ(stats.errors, 0);
}

TEST_F(DirectoryOrganizerTest, NoMatchingRules) {
    // Create files that don't match any rules
    createTestFile(sourceDir / "file1.xyz");
    createTestFile(sourceDir / "file2.abc");
    
    // Create organizer with only specific rules (no catch-all)
    std::vector<std::unique_ptr<ISortingRule>> limitedRules;
    auto pdfRule = std::make_unique<ConfigurableRule>("documents/pdf", 10);
    pdfRule->addCondition(std::make_unique<ExtensionCondition>(".pdf"));
    limitedRules.push_back(std::move(pdfRule));
    
    DirectoryOrganizer organizer(sourceDir, targetDir, std::move(limitedRules), false);
    organizer.scanAndOrganize();
    
    // Files should be processed but skipped (no matching rules)
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 2);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 0);
    EXPECT_EQ(stats.filesSkipped, 2);
    EXPECT_EQ(stats.errors, 0);
    
    // Files should still be in source
    EXPECT_TRUE(std::filesystem::exists(sourceDir / "file1.xyz"));
    EXPECT_TRUE(std::filesystem::exists(sourceDir / "file2.abc"));
}

TEST_F(DirectoryOrganizerTest, NestedDirectoryStructure) {
    // Create nested directory structure
    createTestFile(sourceDir / "level1/level2/document.pdf");
    createTestFile(sourceDir / "level1/text.txt");
    createTestFile(sourceDir / "level1/level2/level3/deep.pdf");
    
    // Verify files were created before processing
    EXPECT_TRUE(std::filesystem::exists(sourceDir / "level1/level2/document.pdf"));
    EXPECT_TRUE(std::filesystem::exists(sourceDir / "level1/text.txt"));
    EXPECT_TRUE(std::filesystem::exists(sourceDir / "level1/level2/level3/deep.pdf"));
    
    DirectoryOrganizer organizer(sourceDir, targetDir, copyRules(), false);
    organizer.scanAndOrganize();
    
    // The entire level1 directory gets moved by the catch-all rule
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.directoriesProcessed, 1);
    EXPECT_EQ(stats.directoriesMovedOrWouldMove, 1);
    
    // Verify the directory was moved with its contents intact
    EXPECT_TRUE(std::filesystem::exists(targetDir / "others/level1/level2/document.pdf"));
    EXPECT_TRUE(std::filesystem::exists(targetDir / "others/level1/text.txt"));
    EXPECT_TRUE(std::filesystem::exists(targetDir / "others/level1/level2/level3/deep.pdf"));
}

TEST_F(DirectoryOrganizerTest, IndividualFileProcessingFromNestedDirs) {
    // Create files directly in source directory (no subdirectories to avoid directory moves)
    createTestFile(sourceDir / "document1.pdf");
    createTestFile(sourceDir / "text1.txt");
    createTestFile(sourceDir / "document2.pdf");
    createTestFile(sourceDir / "text2.txt");
    createTestFile(sourceDir / "unknown.xyz");
    
    DirectoryOrganizer organizer(sourceDir, targetDir, copyRules(), false);
    organizer.scanAndOrganize();
    
    // Verify all files were processed individually
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 5);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 5);
    EXPECT_EQ(stats.directoriesProcessed, 0);  // No directories to process
    
    // Verify file locations
    EXPECT_TRUE(std::filesystem::exists(targetDir / "documents/pdf/document1.pdf"));
    EXPECT_TRUE(std::filesystem::exists(targetDir / "documents/pdf/document2.pdf"));
    EXPECT_TRUE(std::filesystem::exists(targetDir / "documents/text/text1.txt"));
    EXPECT_TRUE(std::filesystem::exists(targetDir / "documents/text/text2.txt"));
    EXPECT_TRUE(std::filesystem::exists(targetDir / "others/unknown.xyz"));
}

TEST_F(DirectoryOrganizerTest, RulePriorityRespected) {
    // Create a file that could match multiple rules
    createTestFile(sourceDir / "test.pdf");
    
    // Create rules with different priorities for PDF files
    std::vector<std::unique_ptr<ISortingRule>> priorityRules;
    
    // Lower priority rule (higher number)
    auto lowPriorityRule = std::make_unique<ConfigurableRule>("low_priority", 100);
    lowPriorityRule->addCondition(std::make_unique<ExtensionCondition>(".pdf"));
    priorityRules.push_back(std::move(lowPriorityRule));
    
    // Higher priority rule (lower number)
    auto highPriorityRule = std::make_unique<ConfigurableRule>("high_priority", 5);
    highPriorityRule->addCondition(std::make_unique<ExtensionCondition>(".pdf"));
    priorityRules.push_back(std::move(highPriorityRule));
    
    DirectoryOrganizer organizer(sourceDir, targetDir, std::move(priorityRules), false);
    organizer.scanAndOrganize();
    
    // File should go to high priority rule
    EXPECT_TRUE(std::filesystem::exists(targetDir / "high_priority/test.pdf"));
    EXPECT_FALSE(std::filesystem::exists(targetDir / "low_priority/test.pdf"));
}

TEST_F(DirectoryOrganizerTest, MultipleRunsWithSameOrganizer) {
    DirectoryOrganizer organizer(sourceDir, targetDir, copyRules(), false);
    
    // First run
    createTestFile(sourceDir / "file1.pdf");
    organizer.scanAndOrganize();
    
    auto stats1 = organizer.getStatistics();
    EXPECT_EQ(stats1.filesProcessed, 1);
    
    // Second run (add more files)
    createTestFile(sourceDir / "file2.txt");
    createTestFile(sourceDir / "file3.pdf");
    organizer.scanAndOrganize();
    
    auto stats2 = organizer.getStatistics();
    // Statistics should reflect the second run only
    EXPECT_EQ(stats2.filesProcessed, 2);
    EXPECT_EQ(stats2.filesMovedOrWouldMove, 2);
}

TEST_F(DirectoryOrganizerTest, SkipItemsInTargetDirectory) {
    // Create files in source
    createTestFile(sourceDir / "source_file.pdf");
    
    // Create files already in target directory structure
    createTestFile(targetDir / "documents/pdf/existing_file.pdf");
    
    DirectoryOrganizer organizer(sourceDir, targetDir, copyRules(), false);
    organizer.scanAndOrganize();
    
    // Should process source file but not target file
    const auto& stats = organizer.getStatistics();
    EXPECT_EQ(stats.filesProcessed, 1);
    EXPECT_EQ(stats.filesMovedOrWouldMove, 1);
    
    // Both files should exist in target
    EXPECT_TRUE(std::filesystem::exists(targetDir / "documents/pdf/source_file.pdf"));
    EXPECT_TRUE(std::filesystem::exists(targetDir / "documents/pdf/existing_file.pdf"));
} 