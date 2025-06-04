#pragma once

#include <utility>
#include <string>
#include <stdexcept>

// Template class to store rule parameters of various types in a type-safe manner.
template<typename T>
class RuleParameter {
public:
    // Constructor with value (attribute of type T)
    explicit RuleParameter(T val) : value(std::move(val)) {}
    
    // Default constructor
    RuleParameter() = default;
    
    // Copy constructor
    RuleParameter(const RuleParameter& other) : value(other.value) {}
    
    // Move constructor
    RuleParameter(RuleParameter&& other) noexcept : value(std::move(other.value)) {}
    
    // Copy assignment
    RuleParameter& operator=(const RuleParameter& other) {
        if (this != &other) {
            value = other.value;
        }
        return *this;
    }
    
    // Move assignment
    RuleParameter& operator=(RuleParameter&& other) noexcept {
        if (this != &other) {
            value = std::move(other.value);
        }
        return *this;
    }
    
    const T& getValue() const { return value; }
    
    void setValue(const T& newValue) { value = newValue; }
    void setValue(T&& newValue) { value = std::move(newValue); }
    
    // Template member function for comparison with different types
    template<typename U>
    bool isEqual(const U& other) const {
        if constexpr (std::is_convertible_v<U, T>) {
            return value == static_cast<T>(other);
        }
        return false;
    }
    
    // Comparison operators
    bool operator==(const RuleParameter& other) const {
        return value == other.value;
    }
    
    bool operator!=(const RuleParameter& other) const {
        return !(*this == other);
    }
    
    bool operator<(const RuleParameter& other) const {
        return value < other.value;
    }
    
    bool operator<=(const RuleParameter& other) const {
        return value <= other.value;
    }
    
    bool operator>(const RuleParameter& other) const {
        return value > other.value;
    }
    
    bool operator>=(const RuleParameter& other) const {
        return value >= other.value;
    }
    
    // String representation for debugging
    std::string toString() const {
        if constexpr (std::is_same_v<T, std::string>) {
            return value;
        } else if constexpr (std::is_arithmetic_v<T>) {
            return std::to_string(value);
        } else {
            return "RuleParameter[complex type]";
        }
    }

private:
    T value{};
}; 