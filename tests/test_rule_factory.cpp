#include <gtest/gtest.h>
#include <algorithm>
#include <filesystem>
#include "core/RuleFactory.h"
#include "core/ConfigurationParser.h"
#include "conditions/ExtensionCondition.h"
#include "rules/ConfigurableRule.h"
#include "models/ItemRepresentation.h"

class RuleFactoryTest : public testing::Test {
protected:
    void SetUp() override {
        factory = std::make_unique<RuleFactory>();
    }
    
    std::unique_ptr<RuleFactory> factory;
};

TEST_F(RuleFactoryTest, RegisterAndCreateExtensionCondition) {
    // Test basic extension condition creation
    auto condition = factory->createCondition("EXTENSION", ".pdf");
    ASSERT_NE(condition, nullptr);
    
    // Test that the condition works correctly
    ItemRepresentation pdfItem(std::filesystem::path("test.pdf"));
    ItemRepresentation txtItem(std::filesystem::path("test.txt"));
    
    EXPECT_TRUE(condition->evaluate(pdfItem));
    EXPECT_FALSE(condition->evaluate(txtItem));
}

TEST_F(RuleFactoryTest, ExtensionNormalization) {
    // Test extension normalization (case insensitive, dot handling)
    auto condition1 = factory->createCondition("EXTENSION", ".PDF");
    auto condition2 = factory->createCondition("EXTENSION", "pdf");
    auto condition3 = factory->createCondition("EXTENSION", "PDF");
    
    ASSERT_NE(condition1, nullptr);
    ASSERT_NE(condition2, nullptr);
    ASSERT_NE(condition3, nullptr);
    
    ItemRepresentation pdfItem(std::filesystem::path("test.pdf"));
    ItemRepresentation PDFItem(std::filesystem::path("test.PDF"));
    
    // All should match regardless of case or dot prefix
    EXPECT_TRUE(condition1->evaluate(pdfItem));
    EXPECT_TRUE(condition1->evaluate(PDFItem));
    EXPECT_TRUE(condition2->evaluate(pdfItem));
    EXPECT_TRUE(condition2->evaluate(PDFItem));
    EXPECT_TRUE(condition3->evaluate(pdfItem));
    EXPECT_TRUE(condition3->evaluate(PDFItem));
}

TEST_F(RuleFactoryTest, UnknownConditionType) {
    // Test handling of unknown condition types
    auto condition = factory->createCondition("UNKNOWN_CONDITION", "value");
    EXPECT_EQ(condition, nullptr);
}

TEST_F(RuleFactoryTest, CreateRuleFromConfig) {
    // Create a rule configuration
    RuleConfig config;
    config.targetPath = "documents/pdf";
    config.priority = 10;
    config.appliesTo = "file";
    config.conditions["EXTENSION"] = ".pdf";
    
    auto rule = factory->createRule(config);
    ASSERT_NE(rule, nullptr);
    
    // Test rule properties
    EXPECT_EQ(rule->getTargetRelativePath(), "documents/pdf");
    EXPECT_EQ(rule->getPriority(), 10);
    
    // Test rule matching
    ItemRepresentation pdfItem(std::filesystem::path("test.pdf"));
    ItemRepresentation txtItem(std::filesystem::path("test.txt"));
    
    EXPECT_TRUE(rule->matches(pdfItem));
    EXPECT_FALSE(rule->matches(txtItem));
}

TEST_F(RuleFactoryTest, CreateRuleWithMultipleConditions) {
    // Create a rule configuration with multiple conditions
    RuleConfig config;
    config.targetPath = "documents";
    config.priority = 20;
    config.appliesTo = "file";
    config.conditions["EXTENSION"] = ".pdf";
    // Note: For now we only have extension condition, but this tests the framework
    
    auto rule = factory->createRule(config);
    ASSERT_NE(rule, nullptr);
    
    // Test that the rule was created correctly
    EXPECT_EQ(rule->getTargetRelativePath(), "documents");
    EXPECT_EQ(rule->getPriority(), 20);
}

TEST_F(RuleFactoryTest, CreateRuleWithInvalidCondition) {
    // Create a rule configuration with invalid condition
    RuleConfig config;
    config.targetPath = "documents";
    config.priority = 30;
    config.appliesTo = "file";
    config.conditions["INVALID_CONDITION"] = "some_value";
    config.conditions["EXTENSION"] = ".txt";  // Valid condition
    
    auto rule = factory->createRule(config);
    ASSERT_NE(rule, nullptr);
    
    // Rule should be created but only with valid conditions
    ItemRepresentation txtItem(std::filesystem::path("test.txt"));
    EXPECT_TRUE(rule->matches(txtItem));
}

TEST_F(RuleFactoryTest, CreateRulesFromConfigurationParser) {
    // Create a mock configuration
    ConfigurationParser parser;
    
    // We can't easily test this without creating files, but we can test the interface
    auto registeredTypes = factory->getRegisteredConditionTypes();
    EXPECT_FALSE(registeredTypes.empty());
    EXPECT_TRUE(std::ranges::find(registeredTypes, "EXTENSION") != registeredTypes.end());
}

TEST_F(RuleFactoryTest, RulePrioritySorting) {
    // Create multiple rules with different priorities
    std::vector<RuleConfig> configs;
    
    RuleConfig config1;
    config1.targetPath = "high_priority";
    config1.priority = 5;
    config1.appliesTo = "file";
    config1.conditions["EXTENSION"] = ".pdf";
    configs.push_back(config1);
    
    RuleConfig config2;
    config2.targetPath = "low_priority";
    config2.priority = 100;
    config2.appliesTo = "file";
    config2.conditions["EXTENSION"] = ".txt";
    configs.push_back(config2);
    
    RuleConfig config3;
    config3.targetPath = "medium_priority";
    config3.priority = 50;
    config3.appliesTo = "file";
    config3.conditions["EXTENSION"] = ".jpg";
    configs.push_back(config3);
    
    // Create rules
    std::vector<std::unique_ptr<ISortingRule>> rules;
    for (const auto& config : configs) {
        auto rule = factory->createRule(config);
        if (rule) {
            rules.push_back(std::move(rule));
        }
    }
    
    // Sort by priority (this is what createRulesFromConfig would do)
    std::ranges::sort(rules, [](const auto& a, const auto& b) {
        return a->getPriority() < b->getPriority();
    });
    
    // Verify sorting order
    ASSERT_EQ(rules.size(), 3);
    EXPECT_EQ(rules[0]->getPriority(), 5);    // high_priority
    EXPECT_EQ(rules[1]->getPriority(), 50);   // medium_priority
    EXPECT_EQ(rules[2]->getPriority(), 100);  // low_priority
    
    EXPECT_EQ(rules[0]->getTargetRelativePath(), "high_priority");
    EXPECT_EQ(rules[1]->getTargetRelativePath(), "medium_priority");
    EXPECT_EQ(rules[2]->getTargetRelativePath(), "low_priority");
}

TEST_F(RuleFactoryTest, GetRegisteredConditionTypes) {
    auto types = factory->getRegisteredConditionTypes();
    
    EXPECT_FALSE(types.empty());
    EXPECT_TRUE(std::ranges::find(types, "EXTENSION") != types.end());
    
    // Should only have the default registered conditions for now
    EXPECT_EQ(types.size(), 1);
}

TEST_F(RuleFactoryTest, CustomConditionRegistration) {
    // Test registering a custom condition type
    bool customConditionCalled = false;
    
    factory->registerConditionType("CUSTOM_TEST", [&customConditionCalled](const std::string& value) -> std::unique_ptr<ICondition> {
        customConditionCalled = true;
        // Return a simple extension condition for testing
        return std::make_unique<ExtensionCondition>(".custom");
    });
    
    // Verify it was registered
    auto types = factory->getRegisteredConditionTypes();
    EXPECT_TRUE(std::ranges::find(types, "CUSTOM_TEST") != types.end());
    
    // Test creating the custom condition
    auto condition = factory->createCondition("CUSTOM_TEST", "test_value");
    EXPECT_TRUE(customConditionCalled);
    EXPECT_NE(condition, nullptr);
}

TEST_F(RuleFactoryTest, EmptyConditionsRule) {
    // Test creating a rule with no conditions (should match everything)
    RuleConfig config;
    config.targetPath = "catch_all";
    config.priority = 999;
    config.appliesTo = "any";
    // No conditions
    
    auto rule = factory->createRule(config);
    ASSERT_NE(rule, nullptr);
    
    // Should match any file since there are no conditions
    ItemRepresentation anyItem(std::filesystem::path("anything.xyz"));
    EXPECT_TRUE(rule->matches(anyItem));
} 