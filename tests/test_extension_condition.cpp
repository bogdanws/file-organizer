#include <gtest/gtest.h>
#include "conditions/ExtensionCondition.h"
#include "models/ItemRepresentation.h"
#include <filesystem>
#include <fstream>

class ExtensionConditionTest : public testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for testing
        testDir = std::filesystem::temp_directory_path() / "extension_condition_test";
        std::filesystem::create_directories(testDir);
        
        // Create test files with different extensions
        txtFile = testDir / "test.txt";
        pdfFile = testDir / "document.PDF";  // Test case insensitivity
        noExtFile = testDir / "no_extension";
        testSubDir = testDir / "subdirectory";
        
        // Create actual files/directories
        std::ofstream(txtFile) << "Text content";
        std::ofstream(pdfFile) << "PDF content";
        std::ofstream(noExtFile) << "No extension content";
        std::filesystem::create_directory(testSubDir);
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(testDir);
    }
    
    std::filesystem::path testDir;
    std::filesystem::path txtFile;
    std::filesystem::path pdfFile;
    std::filesystem::path noExtFile;
    std::filesystem::path testSubDir;
};

TEST_F(ExtensionConditionTest, MatchingExtensionWithDot) {
    ExtensionCondition condition(".txt");
    ItemRepresentation item(txtFile);
    
    EXPECT_TRUE(condition.evaluate(item));
    EXPECT_EQ(condition.describe(), "Extension equals '.txt'");
}

TEST_F(ExtensionConditionTest, MatchingExtensionWithoutDot) {
    ExtensionCondition condition("txt");  // Should automatically add dot
    ItemRepresentation item(txtFile);
    
    EXPECT_TRUE(condition.evaluate(item));
    EXPECT_EQ(condition.describe(), "Extension equals '.txt'");
}

TEST_F(ExtensionConditionTest, CaseInsensitiveMatching) {
    ExtensionCondition condition(".pdf");
    ItemRepresentation item(pdfFile);  // File has .PDF extension
    
    EXPECT_TRUE(condition.evaluate(item));
}

TEST_F(ExtensionConditionTest, NonMatchingExtension) {
    ExtensionCondition condition(".pdf");
    ItemRepresentation item(txtFile);
    
    EXPECT_FALSE(condition.evaluate(item));
}

TEST_F(ExtensionConditionTest, FileWithoutExtension) {
    ExtensionCondition condition(".txt");
    ItemRepresentation item(noExtFile);
    
    EXPECT_FALSE(condition.evaluate(item));
}

TEST_F(ExtensionConditionTest, DirectoryDoesNotMatch) {
    ExtensionCondition condition(".txt");
    ItemRepresentation item(testSubDir);
    
    EXPECT_FALSE(condition.evaluate(item));  // Directories don't have extensions
}

TEST_F(ExtensionConditionTest, EmptyExtension) {
    ExtensionCondition condition("");
    ItemRepresentation item(noExtFile);
    
    EXPECT_TRUE(condition.evaluate(item));  // Empty extension should match files without extension
}

TEST_F(ExtensionConditionTest, DescribeMethod) {
    ExtensionCondition condition1(".txt");
    ExtensionCondition condition2("pdf");
    
    EXPECT_EQ(condition1.describe(), "Extension equals '.txt'");
    EXPECT_EQ(condition2.describe(), "Extension equals '.pdf'");
} 