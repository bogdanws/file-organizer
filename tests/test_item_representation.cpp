#include <gtest/gtest.h>
#include "models/ItemRepresentation.h"
#include <filesystem>
#include <fstream>

class ItemRepresentationTest : public testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for testing
        testDir = std::filesystem::temp_directory_path() / "file_organizer_test";
        std::filesystem::create_directories(testDir);
        
        // Create test files
        testFile = testDir / "test_file.txt";
        testFileNoExt = testDir / "test_file_no_ext";
        testSubDir = testDir / "test_subdir";
        
        // Create actual files/directories
        std::ofstream(testFile) << "Test content";
        std::ofstream(testFileNoExt) << "Test content without extension";
        std::filesystem::create_directory(testSubDir);
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(testDir);
    }
    
    std::filesystem::path testDir;
    std::filesystem::path testFile;
    std::filesystem::path testFileNoExt;
    std::filesystem::path testSubDir;
};

TEST_F(ItemRepresentationTest, FileWithExtension) {
    ItemRepresentation item(testFile);
    
    EXPECT_TRUE(item.exists());
    EXPECT_EQ(item.getType(), ItemType::File);
    EXPECT_EQ(item.getName(), "test_file.txt");
    EXPECT_EQ(item.getExtension(), ".txt");
    EXPECT_EQ(item.getItemPath(), testFile);
    EXPECT_GT(item.getSizeInBytes(), 0);
}

TEST_F(ItemRepresentationTest, FileWithoutExtension) {
    ItemRepresentation item(testFileNoExt);
    
    EXPECT_TRUE(item.exists());
    EXPECT_EQ(item.getType(), ItemType::File);
    EXPECT_EQ(item.getName(), "test_file_no_ext");
    EXPECT_EQ(item.getExtension(), "");
    EXPECT_GT(item.getSizeInBytes(), 0);
}

TEST_F(ItemRepresentationTest, Directory) {
    ItemRepresentation item(testSubDir);
    
    EXPECT_TRUE(item.exists());
    EXPECT_EQ(item.getType(), ItemType::Directory);
    EXPECT_EQ(item.getName(), "test_subdir");
    EXPECT_EQ(item.getExtension(), "");
    EXPECT_EQ(item.getSizeInBytes(), 0);  // Directories have size 0 in our implementation
}

TEST_F(ItemRepresentationTest, NonExistentItem) {
    std::filesystem::path nonExistent = testDir / "non_existent_file.txt";
    ItemRepresentation item(nonExistent);
    
    EXPECT_FALSE(item.exists());
    EXPECT_EQ(item.getType(), ItemType::File);  // Non-existent items with extensions are assumed to be files
    EXPECT_EQ(item.getExtension(), ".txt");  // Extension should be parsed from path
    EXPECT_EQ(item.getSizeInBytes(), 0);
}

TEST_F(ItemRepresentationTest, LastModifiedTime) {
    ItemRepresentation item(testFile);
    
    EXPECT_TRUE(item.exists());
    
    // The last modified time should be recent (within the last minute)
    auto now = std::filesystem::file_time_type::clock::now();
    auto itemTime = item.getLastModifiedDate();
    auto diff = std::chrono::duration_cast<std::chrono::seconds>(now - itemTime);
    
    // Should be modified within the last 60 seconds (generous for test environment)
    EXPECT_LT(diff.count(), 60);
} 