#include <gtest/gtest.h>
#include "conditions/AgeCondition.h"
#include "models/ItemRepresentation.h"
#include <filesystem>
#include <fstream>
#include <chrono>
#include <thread>

class AgeConditionTest : public ::testing::Test {
protected:
    void SetUp() override {
        // create a temporary test directory
        testDir = std::filesystem::temp_directory_path() / "age_condition_test";
        std::filesystem::create_directories(testDir);
        
        // create test files
        oldFile = testDir / "old.txt";
        newFile = testDir / "new.txt";
        
        // create old file
        {
            std::ofstream file(oldFile);
            file << "old content";
        }
        
        // wait a bit to ensure time difference
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // create new file
        {
            std::ofstream file(newFile);
            file << "new content";
        }
    }
    
    void TearDown() override {
        // clean up test files
        std::filesystem::remove_all(testDir);
    }
    
    std::filesystem::path testDir;
    std::filesystem::path oldFile;
    std::filesystem::path newFile;
};

TEST_F(AgeConditionTest, ConstructorAndGetters) {
    auto duration = std::chrono::hours(24);
    AgeCondition condition(AgeComparison::OlderThan, duration);
    
    EXPECT_EQ(condition.getThreshold(), duration);
    EXPECT_EQ(condition.getComparison(), AgeComparison::OlderThan);
}

TEST_F(AgeConditionTest, OlderThanCondition) {
    // test with very short threshold (should match most files)
    auto shortDuration = std::chrono::milliseconds(50);
    AgeCondition condition(AgeComparison::OlderThan, shortDuration);
    
    ItemRepresentation oldItem(oldFile);
    ItemRepresentation newItem(newFile);
    
    // both files should be older than 50ms given our sleep
    EXPECT_TRUE(condition.evaluate(oldItem));
    // new file might or might not be older than 50ms, but old file definitely should be
}

TEST_F(AgeConditionTest, NewerThanCondition) {
    // test with very long threshold (should match all recent files)
    auto longDuration = std::chrono::hours(24);
    AgeCondition condition(AgeComparison::NewerThan, longDuration);
    
    ItemRepresentation oldItem(oldFile);
    ItemRepresentation newItem(newFile);
    
    // both files should be newer than 24 hours
    EXPECT_TRUE(condition.evaluate(oldItem));
    EXPECT_TRUE(condition.evaluate(newItem));
}

TEST_F(AgeConditionTest, DirectoryHandling) {
    auto duration = std::chrono::hours(1);
    AgeCondition condition(AgeComparison::OlderThan, duration);
    
    ItemRepresentation dirItem(testDir);
    
    // directories should be handled the same as files for age
    // just test that it doesn't crash
    bool result = condition.evaluate(dirItem);
    // don't assert specific value since it depends on timing
    (void)result; // suppress unused variable warning
}

TEST_F(AgeConditionTest, TemplateSetAgeThreshold) {
    auto duration = std::chrono::hours(24);
    AgeCondition condition(AgeComparison::OlderThan, duration);
    
    // test template member function with different duration types
    auto newDuration = std::chrono::minutes(60); // 1 hour
    condition.setAgeThreshold(newDuration);
    
    auto result = condition.getThreshold();
    auto expected = std::chrono::duration_cast<std::chrono::system_clock::duration>(newDuration);
    EXPECT_EQ(result, expected);
    
    // test with seconds
    auto secondsDuration = std::chrono::seconds(3600); // also 1 hour
    condition.setAgeThreshold(secondsDuration);
    
    auto result2 = condition.getThreshold();
    auto expected2 = std::chrono::duration_cast<std::chrono::system_clock::duration>(secondsDuration);
    EXPECT_EQ(result2, expected2);
}

TEST_F(AgeConditionTest, DescribeMethod) {
    auto hourDuration = std::chrono::hours(2);
    auto dayDuration = std::chrono::hours(24 * 5); // 5 days
    auto monthDuration = std::chrono::hours(24 * 30 * 2); // 2 months
    auto yearDuration = std::chrono::hours(24 * 365 * 3); // 3 years
    
    AgeCondition hourCondition(AgeComparison::OlderThan, hourDuration);
    AgeCondition dayCondition(AgeComparison::NewerThan, dayDuration);
    AgeCondition monthCondition(AgeComparison::OlderThan, monthDuration);
    AgeCondition yearCondition(AgeComparison::NewerThan, yearDuration);
    
    std::string hourDesc = hourCondition.describe();
    std::string dayDesc = dayCondition.describe();
    std::string monthDesc = monthCondition.describe();
    std::string yearDesc = yearCondition.describe();
    
    EXPECT_TRUE(hourDesc.find("older than") != std::string::npos);
    EXPECT_TRUE(hourDesc.find("hour") != std::string::npos);
    
    EXPECT_TRUE(dayDesc.find("newer than") != std::string::npos);
    EXPECT_TRUE(dayDesc.find("day") != std::string::npos);
    
    EXPECT_TRUE(monthDesc.find("older than") != std::string::npos);
    EXPECT_TRUE(monthDesc.find("month") != std::string::npos);
    
    EXPECT_TRUE(yearDesc.find("newer than") != std::string::npos);
    EXPECT_TRUE(yearDesc.find("year") != std::string::npos);
}

TEST_F(AgeConditionTest, EdgeCases) {
    // test with zero threshold
    auto zeroDuration = std::chrono::milliseconds(0);
    AgeCondition zeroCondition(AgeComparison::OlderThan, zeroDuration);
    ItemRepresentation item(newFile);
    
    // file should be older than 0 duration
    EXPECT_TRUE(zeroCondition.evaluate(item));
    
    // test with very large threshold
    auto largeDuration = std::chrono::hours(24 * 365 * 100); // 100 years
    AgeCondition largeCondition(AgeComparison::NewerThan, largeDuration);
    EXPECT_TRUE(largeCondition.evaluate(item)); // file should be newer than 100 years
} 