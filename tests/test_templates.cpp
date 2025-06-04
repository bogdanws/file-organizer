#include <gtest/gtest.h>
#include "core/RuleParameter.h"
#include "core/ValueParser.h"
#include "conditions/SizeCondition.h"
#include "conditions/AgeCondition.h"
#include <chrono>

class TemplateTest : public ::testing::Test {
protected:
    void SetUp() override {
        // setup code if needed
    }
};

// Test RuleParameter template class
TEST_F(TemplateTest, RuleParameterBasicOperations) {
    // test with integer type
    RuleParameter<int> intParam(42);
    EXPECT_EQ(intParam.getValue(), 42);
    
    // test setter
    intParam.setValue(100);
    EXPECT_EQ(intParam.getValue(), 100);
    
    // test with string type
    RuleParameter<std::string> stringParam("test");
    EXPECT_EQ(stringParam.getValue(), "test");
    
    // test copy constructor
    RuleParameter<int> copiedParam(intParam);
    EXPECT_EQ(copiedParam.getValue(), 100);
    
    // test move constructor
    RuleParameter<std::string> movedParam(std::move(stringParam));
    EXPECT_EQ(movedParam.getValue(), "test");
}

TEST_F(TemplateTest, RuleParameterComparisons) {
    RuleParameter<int> param1(10);
    RuleParameter<int> param2(20);
    RuleParameter<int> param3(10);
    
    EXPECT_TRUE(param1 == param3);
    EXPECT_FALSE(param1 == param2);
    EXPECT_TRUE(param1 != param2);
    EXPECT_TRUE(param1 < param2);
    EXPECT_TRUE(param2 > param1);
    EXPECT_TRUE(param1 <= param3);
    EXPECT_TRUE(param1 >= param3);
}

TEST_F(TemplateTest, RuleParameterTemplateMethod) {
    RuleParameter<int> param(10);
    
    // test template member function isEqual with different types
    EXPECT_TRUE(param.isEqual(10));
    EXPECT_TRUE(param.isEqual(10.0));
    EXPECT_FALSE(param.isEqual(20));
    EXPECT_TRUE(param.isEqual(10.5)); // 10.5 casts to 10, so this should be true
    
    // test with different value to ensure false case works
    EXPECT_FALSE(param.isEqual(11.5)); // 11.5 casts to 11, which != 10
}

TEST_F(TemplateTest, RuleParameterToString) {
    RuleParameter<int> intParam(42);
    EXPECT_EQ(intParam.toString(), "42");
    
    RuleParameter<std::string> stringParam("hello");
    EXPECT_EQ(stringParam.toString(), "hello");
    
    RuleParameter<double> doubleParam(3.14);
    EXPECT_EQ(doubleParam.toString(), "3.140000");
}

// Test external template functions
TEST_F(TemplateTest, ParseValueString) {
    std::string result = parseValue<std::string>("hello world");
    EXPECT_EQ(result, "hello world");
}

TEST_F(TemplateTest, ParseValueInt) {
    int result = parseValue<int>("123");
    EXPECT_EQ(result, 123);
    
    EXPECT_THROW(parseValue<int>("not_a_number"), std::invalid_argument);
}

TEST_F(TemplateTest, ParseValueBool) {
    EXPECT_TRUE(parseValue<bool>("true"));
    EXPECT_TRUE(parseValue<bool>("TRUE"));
    EXPECT_TRUE(parseValue<bool>("1"));
    EXPECT_TRUE(parseValue<bool>("yes"));
    EXPECT_TRUE(parseValue<bool>("on"));
    
    EXPECT_FALSE(parseValue<bool>("false"));
    EXPECT_FALSE(parseValue<bool>("FALSE"));
    EXPECT_FALSE(parseValue<bool>("0"));
    EXPECT_FALSE(parseValue<bool>("no"));
    EXPECT_FALSE(parseValue<bool>("off"));
    
    EXPECT_THROW(parseValue<bool>("maybe"), std::invalid_argument);
}

TEST_F(TemplateTest, ParseValueSize) {
    // test basic bytes
    EXPECT_EQ(parseValue<std::uintmax_t>("100"), 100);
    EXPECT_EQ(parseValue<std::uintmax_t>("100b"), 100);
    
    // test KB
    EXPECT_EQ(parseValue<std::uintmax_t>("1kb"), 1024);
    EXPECT_EQ(parseValue<std::uintmax_t>("2KB"), 2048);
    
    // test MB
    EXPECT_EQ(parseValue<std::uintmax_t>("1mb"), 1024 * 1024);
    EXPECT_EQ(parseValue<std::uintmax_t>("5MB"), 5 * 1024 * 1024);
    
    // test GB
    EXPECT_EQ(parseValue<std::uintmax_t>("1gb"), 1024ULL * 1024 * 1024);
    EXPECT_EQ(parseValue<std::uintmax_t>("2GB"), 2ULL * 1024 * 1024 * 1024);
    
    // test invalid formats
    EXPECT_THROW(parseValue<std::uintmax_t>(""), std::invalid_argument);
    EXPECT_THROW(parseValue<std::uintmax_t>("abc"), std::invalid_argument);
    EXPECT_THROW(parseValue<std::uintmax_t>("100xyz"), std::invalid_argument);
}

TEST_F(TemplateTest, ParseValueDuration) {
    // test days
    auto duration1 = parseValue<std::chrono::system_clock::duration>("1d");
    auto expected1 = std::chrono::hours(24);
    EXPECT_EQ(std::chrono::duration_cast<std::chrono::hours>(duration1), expected1);
    
    // test months (approximate)
    auto duration2 = parseValue<std::chrono::system_clock::duration>("1m");
    auto expected2 = std::chrono::hours(24 * 30);
    EXPECT_EQ(std::chrono::duration_cast<std::chrono::hours>(duration2), expected2);
    
    // test years (approximate)
    auto duration3 = parseValue<std::chrono::system_clock::duration>("1y");
    auto expected3 = std::chrono::hours(24 * 365);
    EXPECT_EQ(std::chrono::duration_cast<std::chrono::hours>(duration3), expected3);
    
    // test multiple units
    auto duration4 = parseValue<std::chrono::system_clock::duration>("30d");
    auto expected4 = std::chrono::hours(30 * 24);
    EXPECT_EQ(std::chrono::duration_cast<std::chrono::hours>(duration4), expected4);
    
    // test invalid formats
    EXPECT_THROW(parseValue<std::chrono::system_clock::duration>(""), std::invalid_argument);
    EXPECT_THROW(parseValue<std::chrono::system_clock::duration>("abc"), std::invalid_argument);
    EXPECT_THROW(parseValue<std::chrono::system_clock::duration>("100x"), std::invalid_argument);
}

TEST_F(TemplateTest, ParseValueSafe) {
    // test successful parsing
    int result1 = parseValueSafe<int>("123", -1);
    EXPECT_EQ(result1, 123);
    
    // test failed parsing returns default
    int result2 = parseValueSafe<int>("not_a_number", -1);
    EXPECT_EQ(result2, -1);
    
    bool result3 = parseValueSafe<bool>("invalid", false);
    EXPECT_EQ(result3, false);
}

// Test SizeCondition using templates
TEST_F(TemplateTest, SizeConditionWithTemplates) {
    SizeCondition condition(SizeComparison::GreaterThan, 1024 * 1024); // 1MB
    
    // test template member function
    condition.setThreshold(2048ULL);
    EXPECT_EQ(condition.getThreshold(), 2048);
    
    // test with different convertible types
    condition.setThreshold(static_cast<std::uintmax_t>(4096));
    EXPECT_EQ(condition.getThreshold(), 4096);
    
    EXPECT_EQ(condition.getComparison(), SizeComparison::GreaterThan);
}

// Test AgeCondition using templates
TEST_F(TemplateTest, AgeConditionWithTemplates) {
    auto duration = std::chrono::hours(24); // 1 day
    AgeCondition condition(AgeComparison::OlderThan, duration);
    
    // test template member function with different duration types
    auto newDuration = std::chrono::minutes(60); // 1 hour
    condition.setAgeThreshold(newDuration);
    
    auto result = condition.getThreshold();
    auto expected = std::chrono::duration_cast<std::chrono::system_clock::duration>(newDuration);
    EXPECT_EQ(result, expected);
    
    EXPECT_EQ(condition.getComparison(), AgeComparison::OlderThan);
} 