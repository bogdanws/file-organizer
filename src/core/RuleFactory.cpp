#include "RuleFactory.h"
#include "conditions/ExtensionCondition.h"
#include "conditions/SizeCondition.h"
#include "conditions/AgeCondition.h"
#include "rules/ConfigurableRule.h"
#include "core/ValueParser.h"
#include "Logger.h"
#include <algorithm>
#include <ranges>
#include <utility>

RuleFactory::RuleFactory() {
    registerDefaultConditions();
}

void RuleFactory::registerConditionType(const std::string& key, ConditionCreationFunction creator) {
    conditionRegistry[key] = std::move(creator);
    Logger::instance().debug("Registered condition type: " + key);
}

std::unique_ptr<ICondition> RuleFactory::createCondition(const std::string& key, const std::string& value) {
    auto it = conditionRegistry.find(key);
    if (it == conditionRegistry.end()) {
        Logger::instance().warning("Unknown condition type: " + key);
        return nullptr;
    }
    
    try {
        auto condition = it->second(value);
        Logger::instance().debug("Created condition: " + key + " = " + value);
        return condition;
    } catch (const std::exception& e) {
        Logger::instance().error("Failed to create condition " + key + ": " + e.what());
        return nullptr;
    }
}

std::unique_ptr<ISortingRule> RuleFactory::createRule(const RuleConfig& ruleConfig) {
    auto rule = std::make_unique<ConfigurableRule>(
        ruleConfig.targetPath,
        ruleConfig.priority
    );
    
    // add conditions to the rule
    for (const auto& [conditionKey, conditionValue] : ruleConfig.conditions) {
        auto condition = createCondition(conditionKey, conditionValue);
        if (condition) {
            rule->addCondition(std::move(condition));
        } else {
            Logger::instance().warning("Skipping invalid condition: " + conditionKey + " = " + conditionValue);
        }
    }
    
    Logger::instance().info("Created rule: " + ruleConfig.targetPath + " (priority: " + std::to_string(ruleConfig.priority) + ")");
    return rule;
}

std::vector<std::unique_ptr<ISortingRule>> RuleFactory::createRulesFromConfig(const ConfigurationParser& parser) {
    std::vector<std::unique_ptr<ISortingRule>> rules;
    
    for (const auto& ruleConfig : parser.getRules()) {
        auto rule = createRule(ruleConfig);
        if (rule) {
            rules.push_back(std::move(rule));
        }
    }
    
    // sort rules by priority (lower number = higher priority)
    std::ranges::sort(rules, [](const auto& a, const auto& b) {
        return a->getPriority() < b->getPriority();
    });
    
    Logger::instance().info("Created " + std::to_string(rules.size()) + " rules from configuration");
    return rules;
}

std::vector<std::string> RuleFactory::getRegisteredConditionTypes() const {
    std::vector<std::string> types;
    for (const auto &key: conditionRegistry | std::views::keys) {
        types.push_back(key);
    }
    return types;
}

void RuleFactory::registerDefaultConditions() {
    // register extension condition
    registerConditionType("EXTENSION", [](const std::string& value) -> std::unique_ptr<ICondition> {
        std::string normalizedExt = normalizeExtension(value);
        return std::make_unique<ExtensionCondition>(normalizedExt);
    });
    
    // register size conditions using template parsing
    registerConditionType("SIZE_GREATER_THAN", [](const std::string& value) -> std::unique_ptr<ICondition> {
        auto sizeThreshold = parseValue<std::uintmax_t>(value);
        return std::make_unique<SizeCondition>(SizeComparison::GreaterThan, sizeThreshold);
    });
    
    registerConditionType("SIZE_LESS_THAN", [](const std::string& value) -> std::unique_ptr<ICondition> {
        auto sizeThreshold = parseValue<std::uintmax_t>(value);
        return std::make_unique<SizeCondition>(SizeComparison::LessThan, sizeThreshold);
    });
    
    // register age conditions using template parsing
    registerConditionType("AGE_OLDER_THAN", [](const std::string& value) -> std::unique_ptr<ICondition> {
        auto ageThreshold = parseValue<std::chrono::system_clock::duration>(value);
        return std::make_unique<AgeCondition>(AgeComparison::OlderThan, ageThreshold);
    });
    
    registerConditionType("AGE_NEWER_THAN", [](const std::string& value) -> std::unique_ptr<ICondition> {
        auto ageThreshold = parseValue<std::chrono::system_clock::duration>(value);
        return std::make_unique<AgeCondition>(AgeComparison::NewerThan, ageThreshold);
    });
    
    // TODO: Register remaining condition types
    // registerConditionType("NAME_MATCHES", [...]);
    // registerConditionType("IS_EMPTY", [...]);
}

std::string RuleFactory::normalizeExtension(const std::string& extension) {
    std::string normalized = extension;
    
    // ensure extension starts with a dot
    if (!normalized.empty() && normalized[0] != '.') {
        normalized = "." + normalized;
    }
    
    // convert to lowercase for case-insensitive comparison
    std::ranges::transform(normalized, normalized.begin(), tolower);
    
    return normalized;
} 