#include "AgeCondition.h"
#include "core/ValueParser.h"

AgeCondition::AgeCondition(AgeComparison comparison, std::chrono::system_clock::duration threshold)
    : comparisonType(comparison), ageThreshold(threshold) {
}

bool AgeCondition::evaluate(const ItemRepresentation& item) const {
    auto now = std::chrono::system_clock::now();
    auto itemTime = std::chrono::clock_cast<std::chrono::system_clock>(item.getLastModifiedDate());
    auto itemAge = now - itemTime;
    auto threshold = ageThreshold.getValue();
    
    switch (comparisonType) {
        case AgeComparison::OlderThan:
            return itemAge > threshold;
        case AgeComparison::NewerThan:
            return itemAge < threshold;
        default:
            return false;
    }
}

std::string AgeCondition::describe() const {
    std::string comparisonStr;
    switch (comparisonType) {
        case AgeComparison::OlderThan:
            comparisonStr = "older than";
            break;
        case AgeComparison::NewerThan:
            comparisonStr = "newer than";
            break;
    }
    
    // format duration in a human-readable way
    auto threshold = ageThreshold.getValue();
    auto hours = std::chrono::duration_cast<std::chrono::hours>(threshold);
    
    std::string durationStr;
    if (hours.count() >= 24 * 365) {
        int years = static_cast<int>(hours.count() / (24 * 365));
        durationStr = std::to_string(years) + " year" + (years == 1 ? "" : "s");
    } else if (hours.count() >= 24 * 30) {
        int months = static_cast<int>(hours.count() / (24 * 30));
        durationStr = std::to_string(months) + " month" + (months == 1 ? "" : "s");
    } else if (hours.count() >= 24) {
        int days = static_cast<int>(hours.count() / 24);
        durationStr = std::to_string(days) + " day" + (days == 1 ? "" : "s");
    } else {
        durationStr = std::to_string(hours.count()) + " hour" + (hours.count() == 1 ? "" : "s");
    }
    
    return "age " + comparisonStr + " " + durationStr;
} 