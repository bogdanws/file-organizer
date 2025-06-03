#pragma once

#include "models/ItemRepresentation.h"
#include <filesystem>
#include <string>

class ISortingRule {
public:
    virtual ~ISortingRule() = default;
    
    // Check if this rule matches the given item
    virtual bool matches(const ItemRepresentation& item) const = 0;
    
    // Get the target relative path for items matching this rule
    virtual std::filesystem::path getTargetRelativePath() const = 0;
    
    // Get the priority of this rule (lower number = higher priority)
    virtual int getPriority() const = 0;
    
    // Get a description of this rule for logging/debugging
    virtual std::string describe() const = 0;
}; 