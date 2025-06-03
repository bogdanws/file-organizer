#pragma once

#include <memory>
#include <map>
#include <functional>
#include <string>
#include "rules/ISortingRule.h"
#include "conditions/ICondition.h"
#include "ConfigurationParser.h"

class RuleFactory {
public:
    using ConditionCreationFunction = std::function<std::unique_ptr<ICondition>(const std::string& value)>;
    
    RuleFactory();
    ~RuleFactory() = default;
    
    // Register a condition type with its creation function
    void registerConditionType(const std::string& key, ConditionCreationFunction creator);
    
    // Create a condition from configuration data
    std::unique_ptr<ICondition> createCondition(const std::string& key, const std::string& value);
    
    // Create a sorting rule from configuration data
    std::unique_ptr<ISortingRule> createRule(const RuleConfig& ruleConfig);
    
    // Create all rules from configuration parser
    std::vector<std::unique_ptr<ISortingRule>> createRulesFromConfig(const ConfigurationParser& parser);
    
    // Get list of registered condition types
    std::vector<std::string> getRegisteredConditionTypes() const;

private:
    std::map<std::string, ConditionCreationFunction> conditionRegistry;
    
    // Initialize default condition types
    void registerDefaultConditions();
    
    // Helper methods for parsing values
    static std::string normalizeExtension(const std::string& extension);
}; 