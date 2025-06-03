#include "rules/ConfigurableRule.h"
#include <sstream>
#include <utility>

ConfigurableRule::ConfigurableRule(std::filesystem::path targetPath, const int priority)
    : targetRelativePath(std::move(targetPath)), rulePriority(priority) {
}

void ConfigurableRule::addCondition(std::unique_ptr<ICondition> condition) {
    if (condition) {
        conditions.push_back(std::move(condition));
    }
}

bool ConfigurableRule::matches(const ItemRepresentation& item) const {
    // if no conditions, the rule matches everything
    if (conditions.empty()) {
        return true;
    }
    
    // all conditions must be true (AND logic)
    for (const auto& condition : conditions) {
        if (!condition->evaluate(item)) {
            return false;
        }
    }
    
    return true;
}

std::filesystem::path ConfigurableRule::getTargetRelativePath() const {
    return targetRelativePath;
}

int ConfigurableRule::getPriority() const {
    return rulePriority;
}

std::string ConfigurableRule::describe() const {
    std::ostringstream oss;
    oss << "Rule (priority=" << rulePriority << ", target='" << targetRelativePath.string() << "')";
    
    if (!conditions.empty()) {
        oss << " with conditions: ";
        for (size_t i = 0; i < conditions.size(); ++i) {
            if (i > 0) {
                oss << " AND ";
            }
            oss << conditions[i]->describe();
        }
    } else {
        oss << " with no conditions (matches all)";
    }
    
    return oss.str();
} 