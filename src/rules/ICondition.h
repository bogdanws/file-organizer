#pragma once

#include "models/ItemRepresentation.h"
#include <string>

class ICondition {
public:
    virtual ~ICondition() = default;
    
    // Evaluate if the condition is met for the given item
    virtual bool evaluate(const ItemRepresentation& item) const = 0;
    
    // Get a description of this condition for logging/debugging
    virtual std::string describe() const = 0;
}; 