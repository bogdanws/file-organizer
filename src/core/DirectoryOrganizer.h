#pragma once

#include <filesystem>
#include <vector>
#include <memory>
#include "rules/ISortingRule.h"
#include "models/ItemRepresentation.h"

class DirectoryOrganizer {
public:
    DirectoryOrganizer(
        std::filesystem::path sourceDirectory,
        std::filesystem::path targetBaseDirectory,
        std::vector<std::unique_ptr<ISortingRule>> rules,
        bool dryRunEnabled = false
    );
    
    ~DirectoryOrganizer() = default;
    
    // Main method to scan and organize files
    void scanAndOrganize();
    
    // Get statistics about the last operation
    struct Statistics {
        size_t filesProcessed = 0;
        size_t filesMovedOrWouldMove = 0;
        size_t filesSkipped = 0;
        size_t directoriesProcessed = 0;
        size_t directoriesMovedOrWouldMove = 0;
        size_t directoriesSkipped = 0;
        size_t errors = 0;
    };
    
    const Statistics& getStatistics() const { return stats; }
    
    // Reset statistics
    void resetStatistics();

private:
    std::filesystem::path sourceDir;
    std::filesystem::path targetBaseDir;
    std::vector<std::unique_ptr<ISortingRule>> sortingRules;
    bool dryRun;
    Statistics stats;
    
    // Helper methods
    void processItem(const std::filesystem::path& itemPath);
    void processFile(const ItemRepresentation& item);
    void processDirectory(const ItemRepresentation& item);
    
    // Find the first matching rule for an item
    ISortingRule* findMatchingRule(const ItemRepresentation& item) const;
    
    // Move item to target location
    bool moveItem(const ItemRepresentation& item, const std::filesystem::path& targetPath);
    
    // Create directory structure if it doesn't exist
    static bool ensureDirectoryExists(const std::filesystem::path& directory);
    
    // Check if an item should be processed (e.g., not already in target directory)
    bool shouldProcessItem(const ItemRepresentation& item) const;
    
    // Generate unique filename if target already exists
    static std::filesystem::path generateUniqueTarget(const std::filesystem::path& targetPath);
}; 