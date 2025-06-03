#include <gtest/gtest.h>
#include "rules/ConfigurableRule.h"
#include "conditions/ExtensionCondition.h"
#include "models/ItemRepresentation.h"
#include <filesystem>
#include <fstream>

class ConfigurableRuleTest : public testing::Test {
protected:
    void SetUp() override {
        // Create a temporary directory for testing
        testDir = std::filesystem::temp_directory_path() / "configurable_rule_test";
        std::filesystem::create_directories(testDir);
        
        // Create test files
        txtFile = testDir / "test.txt";
        pdfFile = testDir / "document.pdf";
        testSubDir = testDir / "subdirectory";
        
        // Create actual files/directories
        std::ofstream(txtFile) << "Text content";
        std::ofstream(pdfFile) << "PDF content";
        std::filesystem::create_directory(testSubDir);
    }
    
    void TearDown() override {
        // Clean up test files
        std::filesystem::remove_all(testDir);
    }
    
    std::filesystem::path testDir;
    std::filesystem::path txtFile;
    std::filesystem::path pdfFile;
    std::filesystem::path testSubDir;
};

TEST_F(ConfigurableRuleTest, BasicRuleProperties) {
    ConfigurableRule rule("documents/text", 10);
    
    EXPECT_EQ(rule.getTargetRelativePath(), std::filesystem::path("documents/text"));
    EXPECT_EQ(rule.getPriority(), 10);
}

TEST_F(ConfigurableRuleTest, RuleWithoutConditions) {
    ConfigurableRule rule("catch_all", 100);
    ItemRepresentation txtItem(txtFile);
    ItemRepresentation pdfItem(pdfFile);
    ItemRepresentation dirItem(testSubDir);
    
    // Rule without conditions should match everything
    EXPECT_TRUE(rule.matches(txtItem));
    EXPECT_TRUE(rule.matches(pdfItem));
    EXPECT_TRUE(rule.matches(dirItem));
    
    std::string description = rule.describe();
    EXPECT_TRUE(description.find("no conditions") != std::string::npos);
}

TEST_F(ConfigurableRuleTest, RuleWithSingleCondition) {
    ConfigurableRule rule("documents/text", 10);
    rule.addCondition(std::make_unique<ExtensionCondition>(".txt"));
    
    ItemRepresentation txtItem(txtFile);
    ItemRepresentation pdfItem(pdfFile);
    
    EXPECT_TRUE(rule.matches(txtItem));   // Should match .txt file
    EXPECT_FALSE(rule.matches(pdfItem));  // Should not match .pdf file
}

TEST_F(ConfigurableRuleTest, RuleWithMultipleConditions) {
    ConfigurableRule rule("documents/text", 10);
    rule.addCondition(std::make_unique<ExtensionCondition>(".txt"));
    // Note: We'll add more condition types in later steps
    
    ItemRepresentation txtItem(txtFile);
    
    // With only one condition, should still match
    EXPECT_TRUE(rule.matches(txtItem));
}

TEST_F(ConfigurableRuleTest, AddNullCondition) {
    ConfigurableRule rule("test", 10);
    rule.addCondition(nullptr);  // Should handle null gracefully
    
    ItemRepresentation txtItem(txtFile);
    
    // Should still work (no conditions added)
    EXPECT_TRUE(rule.matches(txtItem));
}

TEST_F(ConfigurableRuleTest, DescribeMethod) {
    ConfigurableRule rule("documents/pdf", 20);
    rule.addCondition(std::make_unique<ExtensionCondition>(".pdf"));
    
    std::string description = rule.describe();
    
    EXPECT_TRUE(description.find("priority=20") != std::string::npos);
    EXPECT_TRUE(description.find("documents/pdf") != std::string::npos);
    EXPECT_TRUE(description.find("Extension equals '.pdf'") != std::string::npos);
}

TEST_F(ConfigurableRuleTest, PriorityComparison) {
    ConfigurableRule highPriorityRule("high", 1);
    ConfigurableRule lowPriorityRule("low", 100);
    
    EXPECT_LT(highPriorityRule.getPriority(), lowPriorityRule.getPriority());
}

TEST_F(ConfigurableRuleTest, ComplexTargetPath) {
    ConfigurableRule rule("documents/work/projects/2024", 15);
    
    std::filesystem::path expectedPath = std::filesystem::path("documents") / "work" / "projects" / "2024";
    EXPECT_EQ(rule.getTargetRelativePath(), expectedPath);
} 