#pragma once

#include "ICondition.h"
#include "core/RuleParameter.h"
#include <string>

enum class SizeComparison {
    GreaterThan,
    LessThan
};

class SizeCondition : public ICondition {
public:
    SizeCondition(SizeComparison comparison, std::uintmax_t threshold);
    
    // ICondition interface implementation
    bool evaluate(const ItemRepresentation& item) const override;
    std::string describe() const override;
    
    // Template member function for setting threshold with different units
    template<typename T>
    void setThreshold(const T& value) {
        if constexpr (std::is_convertible_v<T, std::uintmax_t>) {
            sizeThreshold.setValue(static_cast<std::uintmax_t>(value));
        }
    }
    
    std::uintmax_t getThreshold() const { return sizeThreshold.getValue(); }
    
    SizeComparison getComparison() const { return comparisonType; }
    
private:
    SizeComparison comparisonType;
    RuleParameter<std::uintmax_t> sizeThreshold;
}; 