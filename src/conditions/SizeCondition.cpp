#include "SizeCondition.h"
#include "core/ValueParser.h"

SizeCondition::SizeCondition(SizeComparison comparison, std::uintmax_t threshold)
    : comparisonType(comparison), sizeThreshold(threshold) {
}

bool SizeCondition::evaluate(const ItemRepresentation& item) const {
    // only apply to files, not directories
    if (item.getType() != ItemType::File) {
        return false;
    }
    
    std::uintmax_t itemSize = item.getSizeInBytes();
    std::uintmax_t threshold = sizeThreshold.getValue();
    
    switch (comparisonType) {
        case SizeComparison::GreaterThan:
            return itemSize > threshold;
        case SizeComparison::LessThan:
            return itemSize < threshold;
        default:
            return false;
    }
}

std::string SizeCondition::describe() const {
    std::string comparisonStr;
    switch (comparisonType) {
        case SizeComparison::GreaterThan:
            comparisonStr = "greater than";
            break;
        case SizeComparison::LessThan:
            comparisonStr = "less than";
            break;
    }
    
    // format size in a human-readable way
    std::uintmax_t threshold = sizeThreshold.getValue();
    std::string sizeStr;
    
    if (threshold >= 1024ULL * 1024 * 1024) {
        sizeStr = std::to_string(threshold / (1024ULL * 1024 * 1024)) + " GB";
    } else if (threshold >= 1024 * 1024) {
        sizeStr = std::to_string(threshold / (1024 * 1024)) + " MB";
    } else if (threshold >= 1024) {
        sizeStr = std::to_string(threshold / 1024) + " KB";
    } else {
        sizeStr = std::to_string(threshold) + " bytes";
    }
    
    return "size " + comparisonStr + " " + sizeStr;
} 