#include "conditions/ExtensionCondition.h"
#include <algorithm>
#include <utility>

ExtensionCondition::ExtensionCondition(std::string extension)
    : targetExtension(std::move(extension)) {
    // ensure extension starts with a dot
    if (!targetExtension.empty() && targetExtension[0] != '.') {
        targetExtension = "." + targetExtension;
    }
    
    // convert to lowercase for case-insensitive comparison
    std::ranges::transform(targetExtension,
                           targetExtension.begin(), tolower);
}

bool ExtensionCondition::evaluate(const ItemRepresentation& item) const {
    // only files can have extensions
    if (item.getType() != ItemType::File) {
        return false;
    }
    
    std::string itemExtension = item.getExtension();
    
    // convert to lowercase for case-insensitive comparison
    std::ranges::transform(itemExtension,
                           itemExtension.begin(), tolower);
    
    // handle empty extension case
    if (targetExtension.empty() || targetExtension == ".") {
        return itemExtension.empty();
    }
    
    return itemExtension == targetExtension;
}

std::string ExtensionCondition::describe() const {
    return "Extension equals '" + targetExtension + "'";
} 