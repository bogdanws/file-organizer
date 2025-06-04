#pragma once

#include "ICondition.h"
#include "core/RuleParameter.h"
#include <string>
#include <chrono>

enum class AgeComparison {
    OlderThan,
    NewerThan
};

class AgeCondition : public ICondition {
public:
    AgeCondition(AgeComparison comparison, std::chrono::system_clock::duration threshold);
    
    // ICondition interface implementation
    bool evaluate(const ItemRepresentation& item) const override;
    std::string describe() const override;
    
    // Template member function for setting age threshold with different duration types
    template<typename DurationType>
    void setAgeThreshold(const DurationType& duration) {
        auto systemDuration = std::chrono::duration_cast<std::chrono::system_clock::duration>(duration);
        ageThreshold.setValue(systemDuration);
    }
    
    std::chrono::system_clock::duration getThreshold() const { return ageThreshold.getValue(); }
    
    AgeComparison getComparison() const { return comparisonType; }
    
private:
    AgeComparison comparisonType;
    RuleParameter<std::chrono::system_clock::duration> ageThreshold; // Using template class
}; 