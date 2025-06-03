#pragma once

#include "rules/ISortingRule.h"
#include "conditions/ICondition.h"
#include <vector>
#include <memory>
#include <filesystem>

class ConfigurableRule : public ISortingRule {
public:
    ConfigurableRule(std::filesystem::path targetPath, int priority);
    
    // Add a condition to this rule
    void addCondition(std::unique_ptr<ICondition> condition);
    
    // ISortingRule interface implementation
    bool matches(const ItemRepresentation& item) const override;
    std::filesystem::path getTargetRelativePath() const override;
    int getPriority() const override;
    std::string describe() const override;
    
private:
    std::filesystem::path targetRelativePath;
    int rulePriority;
    std::vector<std::unique_ptr<ICondition>> conditions;
}; 