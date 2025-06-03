#pragma once

#include <filesystem>
#include <string>

enum class ItemType {
    File,
    Directory,
    Other
};

class ItemRepresentation {
public:
    // Constructor to populate fields using std::filesystem
    explicit ItemRepresentation(std::filesystem::path  path);
    
    // Getters
    const std::filesystem::path& getItemPath() const { return itemPath; }
    ItemType getType() const { return type; }
    const std::string& getName() const { return name; }
    const std::string& getExtension() const { return extension; }
    std::uintmax_t getSizeInBytes() const { return sizeInBytes; }
    const std::filesystem::file_time_type& getLastModifiedDate() const { return lastModifiedDate; }
    
    // Utility methods
    bool exists() const;
    
private:
    std::filesystem::path itemPath;
    ItemType type;
    std::string name;
    std::string extension;  // empty for directories
    std::uintmax_t sizeInBytes;  // 0 for directories
    std::filesystem::file_time_type lastModifiedDate;
    
    void populateFields();
}; 