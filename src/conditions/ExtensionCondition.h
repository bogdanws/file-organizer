#pragma once

#include "ICondition.h"
#include "core/RuleParameter.h"
#include <string>

class ExtensionCondition : public ICondition {
public:
    explicit ExtensionCondition(const std::string& extension);
    
    // ICondition interface implementation
    bool evaluate(const ItemRepresentation& item) const override;
    std::string describe() const override;
    
    // Template member function for setting extension with different string types
    template<typename T>
    void setExtension(const T& extension) {
        if constexpr (std::is_convertible_v<T, std::string>) {
            targetExtension.setValue(normalizeExtension(static_cast<std::string>(extension)));
        }
    }
    
    // Getter for extension
    const std::string& getExtension() const { return targetExtension.getValue(); }
    
private:
    RuleParameter<std::string> targetExtension;
    
    // Helper method for extension normalization
    static std::string normalizeExtension(const std::string& extension);
}; 