#pragma once

#include "ICondition.h"
#include <string>

class ExtensionCondition : public ICondition {
public:
    explicit ExtensionCondition(std::string extension);
    
    // ICondition interface implementation
    bool evaluate(const ItemRepresentation& item) const override;
    std::string describe() const override;
    
private:
    std::string targetExtension;
}; 