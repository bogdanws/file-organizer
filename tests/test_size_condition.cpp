#include <gtest/gtest.h>
#include "conditions/SizeCondition.h"
#include "models/ItemRepresentation.h"
#include <filesystem>
#include <fstream>

class SizeConditionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // create a temporary test directory
        testDir = std::filesystem::temp_directory_path() / "size_condition_test";
        std::filesystem::create_directories(testDir);
        
        // create test files of different sizes
        smallFile = testDir / "small.txt";
        largeFile = testDir / "large.txt";
        
        // create small file (100 bytes)
        {
            std::ofstream file(smallFile);
            for (int i = 0; i < 100; ++i) {
                file << "x";
            }
        }
        
        // create large file (2KB)
        {
            std::ofstream file(largeFile);
            for (int i = 0; i < 2048; ++i) {
                file << "x";
            }
        }
    }
    
    void TearDown() override {
        // clean up test files
        std::filesystem::remove_all(testDir);
    }
    
    std::filesystem::path testDir;
    std::filesystem::path smallFile;
    std::filesystem::path largeFile;
};

TEST_F(SizeConditionTest, ConstructorAndGetters) {
    SizeCondition condition(SizeComparison::GreaterThan, 1024);
    
    EXPECT_EQ(condition.getThreshold(), 1024);
    EXPECT_EQ(condition.getComparison(), SizeComparison::GreaterThan);
}

TEST_F(SizeConditionTest, GreaterThanCondition) {
    SizeCondition condition(SizeComparison::GreaterThan, 1024); // 1KB threshold
    
    ItemRepresentation smallItem(smallFile);
    ItemRepresentation largeItem(largeFile);
    
    // small file (100 bytes) should not match
    EXPECT_FALSE(condition.evaluate(smallItem));
    
    // large file (2KB) should match
    EXPECT_TRUE(condition.evaluate(largeItem));
}

TEST_F(SizeConditionTest, LessThanCondition) {
    SizeCondition condition(SizeComparison::LessThan, 1024); // 1KB threshold
    
    ItemRepresentation smallItem(smallFile);
    ItemRepresentation largeItem(largeFile);
    
    // small file (100 bytes) should match
    EXPECT_TRUE(condition.evaluate(smallItem));
    
    // large file (2KB) should not match
    EXPECT_FALSE(condition.evaluate(largeItem));
}

TEST_F(SizeConditionTest, DirectoryHandling) {
    SizeCondition condition(SizeComparison::GreaterThan, 0);
    
    ItemRepresentation dirItem(testDir);
    
    // directories should never match size conditions
    EXPECT_FALSE(condition.evaluate(dirItem));
}

TEST_F(SizeConditionTest, TemplateSetThreshold) {
    SizeCondition condition(SizeComparison::GreaterThan, 1024);
    
    // test template member function with different types
    condition.setThreshold(2048ULL);
    EXPECT_EQ(condition.getThreshold(), 2048);
    
    condition.setThreshold(static_cast<std::uintmax_t>(4096));
    EXPECT_EQ(condition.getThreshold(), 4096);
    
    // test with smaller types that are convertible
    condition.setThreshold(1024u);
    EXPECT_EQ(condition.getThreshold(), 1024);
}

TEST_F(SizeConditionTest, DescribeMethod) {
    SizeCondition greaterCondition(SizeComparison::GreaterThan, 1024 * 1024); // 1MB
    SizeCondition lessCondition(SizeComparison::LessThan, 2048); // 2KB
    
    std::string greaterDesc = greaterCondition.describe();
    std::string lessDesc = lessCondition.describe();
    
    EXPECT_TRUE(greaterDesc.find("greater than") != std::string::npos);
    EXPECT_TRUE(greaterDesc.find("MB") != std::string::npos);
    
    EXPECT_TRUE(lessDesc.find("less than") != std::string::npos);
    EXPECT_TRUE(lessDesc.find("KB") != std::string::npos);
}

TEST_F(SizeConditionTest, EdgeCases) {
    // test with zero threshold
    SizeCondition zeroCondition(SizeComparison::GreaterThan, 0);
    ItemRepresentation smallItem(smallFile);
    EXPECT_TRUE(zeroCondition.evaluate(smallItem)); // any file > 0 bytes
    
    // test with very large threshold
    SizeCondition largeCondition(SizeComparison::LessThan, std::numeric_limits<std::uintmax_t>::max());
    EXPECT_TRUE(largeCondition.evaluate(smallItem)); // any file < max size
} 