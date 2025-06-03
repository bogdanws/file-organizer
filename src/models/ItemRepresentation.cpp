#include "models/ItemRepresentation.h"
#include <filesystem>
#include <utility>

ItemRepresentation::ItemRepresentation(std::filesystem::path path)
    : itemPath(std::move(path)), type(ItemType::Other), sizeInBytes(0) {
    populateFields();
}

bool ItemRepresentation::exists() const {
    return std::filesystem::exists(itemPath);
}

void ItemRepresentation::populateFields() {
    // always set name and extension from path, regardless of file existence
    name = itemPath.filename().string();
    
    // get extension from path if it has one
    if (itemPath.has_extension()) {
        extension = itemPath.extension().string();
    } else {
        extension = "";
    }
    
    // if file doesn't exist, make reasonable assumptions based on extension
    if (!exists()) {
        // assume it's a file if it has an extension, directory otherwise
        type = extension.empty() ? ItemType::Directory : ItemType::File;
        sizeInBytes = 0;
        lastModifiedDate = std::filesystem::file_time_type::clock::now();
        return;
    }
    
    // determine type and populate type-specific fields for existing items
    if (std::filesystem::is_regular_file(itemPath)) {
        type = ItemType::File;
        
        // get file size
        std::error_code ec;
        sizeInBytes = std::filesystem::file_size(itemPath, ec);
        if (ec) {
            sizeInBytes = 0;  // set to 0 if unable to get size
        }
    } else if (std::filesystem::is_directory(itemPath)) {
        type = ItemType::Directory;
        extension = "";  // directories don't have extensions
        sizeInBytes = 0;  // directories don't have size in this context
    } else {
        type = ItemType::Other;
        sizeInBytes = 0;
    }
    
    // get last modified time
    std::error_code ec;
    lastModifiedDate = std::filesystem::last_write_time(itemPath, ec);
    if (ec) {
        // if we can't get the time, use current time as fallback
        lastModifiedDate = std::filesystem::file_time_type::clock::now();
    }
} 