#include "conditions/ExtensionCondition.h"
#include <algorithm>
#include <utility>

ExtensionCondition::ExtensionCondition(const std::string& extension)
    : targetExtension(normalizeExtension(extension)) {
}

std::string ExtensionCondition::normalizeExtension(const std::string& extension) {
    std::string normalized = extension;
    
    // ensure extension starts with a dot
    if (!normalized.empty() && normalized[0] != '.') {
        normalized = "." + normalized;
    }
    
    // convert to lowercase for case-insensitive comparison
    std::ranges::transform(normalized, normalized.begin(), tolower);
    
    return normalized;
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
    
    // get the stored extension value
    const std::string& storedExtension = targetExtension.getValue();
    
    // handle empty extension case
    if (storedExtension.empty() || storedExtension == ".") {
        return itemExtension.empty();
    }
    
    return itemExtension == storedExtension;
}

std::string ExtensionCondition::describe() const {
    return "Extension equals '" + targetExtension.getValue() + "'";
} 