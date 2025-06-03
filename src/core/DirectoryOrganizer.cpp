#include "DirectoryOrganizer.h"
#include "Logger.h"
#include <format>
#include <system_error>
#include <sstream>
#include <iomanip>
#include <algorithm>

DirectoryOrganizer::DirectoryOrganizer(
    std::filesystem::path sourceDirectory,
    std::filesystem::path targetBaseDirectory,
    std::vector<std::unique_ptr<ISortingRule>> rules,
    const bool dryRunEnabled
) : sourceDir(std::move(sourceDirectory)),
    targetBaseDir(std::move(targetBaseDirectory)),
    sortingRules(std::move(rules)),
    dryRun(dryRunEnabled) {
    
    // sort rules by priority (lower number = higher priority)
    std::ranges::sort(sortingRules, [](const auto& a, const auto& b) {
        return a->getPriority() < b->getPriority();
    });
    
    resetStatistics();
    
    Logger::instance().info("Initialized DirectoryOrganizer");
    Logger::instance().info("Source directory: " + sourceDir.string());
    Logger::instance().info("Target base directory: " + targetBaseDir.string());
    Logger::instance().info("Number of rules: " + std::to_string(sortingRules.size()));
    Logger::instance().info("Dry run mode: " + std::string(dryRun ? "enabled" : "disabled"));
}

void DirectoryOrganizer::scanAndOrganize() {
    Logger::instance().info("Starting file organization process");
    resetStatistics();
    
    // verify source directory exists
    if (!std::filesystem::exists(sourceDir) || !std::filesystem::is_directory(sourceDir)) {
        Logger::instance().error("Source directory does not exist or is not a directory: " + sourceDir.string());
        stats.errors++;
        return;
    }
    
    // create target base directory if it doesn't exist
    if (!dryRun && !ensureDirectoryExists(targetBaseDir)) {
        Logger::instance().error("Failed to create target base directory: " + targetBaseDir.string());
        stats.errors++;
        return;
    }
    
    try {
        // first collect all items to avoid iterator invalidation during moves
        std::vector<std::filesystem::path> itemsToProcess;
        
        for (const auto& entry : std::filesystem::recursive_directory_iterator(sourceDir)) {
            itemsToProcess.push_back(entry.path());
        }
        
        // now process all collected items
        for (const auto& itemPath : itemsToProcess) {
            try {
                // skip if item no longer exists (might have been moved as part of a directory)
                if (!std::filesystem::exists(itemPath)) {
                    continue;
                }
                processItem(itemPath);
            } catch (const std::exception& e) {
                Logger::instance().error("Error processing item " + itemPath.string() + ": " + e.what());
                stats.errors++;
            }
        }
    } catch (const std::exception& e) {
        Logger::instance().error(std::format("Error scanning source directory: {}", e.what()));
        stats.errors++;
    }
    
    // log final statistics
    Logger::instance().info("Organization process completed");
    Logger::instance().info("Files processed: " + std::to_string(stats.filesProcessed));
    {
        std::ostringstream ss;
        ss << "Files " << (dryRun ? "would be moved" : "moved") << ": " << stats.filesMovedOrWouldMove;
        Logger::instance().info(ss.str());
    }
    Logger::instance().info("Files skipped: " + std::to_string(stats.filesSkipped));
    Logger::instance().info("Directories processed: " + std::to_string(stats.directoriesProcessed));
    {
        std::ostringstream ss;
        ss << "Directories " << (dryRun ? "would be moved" : "moved") << ": " << stats.directoriesMovedOrWouldMove;
        Logger::instance().info(ss.str());
    }
    Logger::instance().info("Directories skipped: " + std::to_string(stats.directoriesSkipped));
    Logger::instance().info("Errors: " + std::to_string(stats.errors));
}

void DirectoryOrganizer::resetStatistics() {
    stats = Statistics{};
}

void DirectoryOrganizer::processItem(const std::filesystem::path& itemPath) {
    try {
        const ItemRepresentation item(itemPath);
        
        if (!shouldProcessItem(item)) {
            return;
        }
        
        if (item.getType() == ItemType::File) {
            processFile(item);
        } else if (item.getType() == ItemType::Directory) {
            processDirectory(item);
        } else {
            Logger::instance().debug("Skipping unsupported item type: " + itemPath.string());
        }
    } catch (const std::exception& e) {
        Logger::instance().error("Failed to create ItemRepresentation for " + itemPath.string() + ": " + e.what());
        stats.errors++;
    }
}

void DirectoryOrganizer::processFile(const ItemRepresentation& item) {
    stats.filesProcessed++;

    const ISortingRule* matchingRule = findMatchingRule(item);
    if (!matchingRule) {
        Logger::instance().debug("No matching rule found for file: " + item.getName());
        stats.filesSkipped++;
        return;
    }
    
    // calculate target path
    const std::filesystem::path targetPath = targetBaseDir / matchingRule->getTargetRelativePath() / item.getName();
    
    Logger::instance().debug("File '" + item.getName() + "' matches rule: " + matchingRule->describe());
    
    if (moveItem(item, targetPath)) {
        stats.filesMovedOrWouldMove++;
        if (dryRun) {
            Logger::instance().info("[DRY RUN] Would move file '" + item.getItemPath().string() + "' to '" + targetPath.string() + "'");
        } else {
            Logger::instance().info("Moved file '" + item.getItemPath().string() + "' to '" + targetPath.string() + "'");
        }
    } else {
        stats.filesSkipped++;
    }
}

void DirectoryOrganizer::processDirectory(const ItemRepresentation& item) {
    stats.directoriesProcessed++;

    const ISortingRule* matchingRule = findMatchingRule(item);
    if (!matchingRule) {
        Logger::instance().debug("No matching rule found for directory: " + item.getName());
        stats.directoriesSkipped++;
        return;
    }
    
    // calculate target path
    const std::filesystem::path targetPath = targetBaseDir / matchingRule->getTargetRelativePath() / item.getName();
    
    Logger::instance().debug("Directory '" + item.getName() + "' matches rule: " + matchingRule->describe());
    
    if (moveItem(item, targetPath)) {
        stats.directoriesMovedOrWouldMove++;
        if (dryRun) {
            Logger::instance().info("[DRY RUN] Would move directory '" + item.getItemPath().string() + "' to '" + targetPath.string() + "'");
        } else {
            Logger::instance().info("Moved directory '" + item.getItemPath().string() + "' to '" + targetPath.string() + "'");
        }
    } else {
        stats.directoriesSkipped++;
    }
}

ISortingRule* DirectoryOrganizer::findMatchingRule(const ItemRepresentation& item) const {
    for (const auto& rule : sortingRules) {
        if (rule->matches(item)) {
            return rule.get();
        }
    }
    return nullptr;
}

bool DirectoryOrganizer::moveItem(const ItemRepresentation& item, const std::filesystem::path& targetPath) {
    if (dryRun) {
        // in dry run mode, just validate the move would be possible
        return true;
    }
    
    try {
        // ensure target directory exists
        if (!ensureDirectoryExists(targetPath.parent_path())) {
            Logger::instance().error("Failed to create target directory: " + targetPath.parent_path().string());
            stats.errors++;
            return false;
        }
        
        // generate unique target if file already exists
        std::filesystem::path finalTargetPath = targetPath;
        if (std::filesystem::exists(targetPath)) {
            finalTargetPath = generateUniqueTarget(targetPath);
            Logger::instance().warning("Target already exists, using: " + finalTargetPath.string());
        }
        
        // perform the move
        std::filesystem::rename(item.getItemPath(), finalTargetPath);
        return true;
        
    } catch (const std::exception& e) {
        Logger::instance().error(std::format("Failed to move item: {}", e.what()));
        stats.errors++;
        return false;
    }
}

bool DirectoryOrganizer::ensureDirectoryExists(const std::filesystem::path& directory) {
    try {
        if (!std::filesystem::exists(directory)) {
            std::filesystem::create_directories(directory);
            Logger::instance().debug("Created directory: " + directory.string());
        }
        return true;
    } catch (const std::exception& e) {
        Logger::instance().error("Failed to create directory " + directory.string() + ": " + e.what());
        return false;
    }
}

bool DirectoryOrganizer::shouldProcessItem(const ItemRepresentation& item) const {
    // skip items that are already in the target directory tree
    const std::filesystem::path itemPath = std::filesystem::absolute(item.getItemPath());
    const std::filesystem::path targetPath = std::filesystem::absolute(targetBaseDir);
    
    // check if item is within target directory
    try {
        std::filesystem::path relativePath = std::filesystem::relative(itemPath, targetPath);
        if (!relativePath.empty() && relativePath.string().find("..") != 0) {
            Logger::instance().debug("Skipping item already in target directory: " + item.getName());
            return false;
        }
    } catch (const std::exception&) {
        // if relative path calculation fails, assume it's safe to process
    }
    
    return true;
}

std::filesystem::path DirectoryOrganizer::generateUniqueTarget(const std::filesystem::path& targetPath) {
    const std::filesystem::path directory = targetPath.parent_path();
    const std::string stem = targetPath.stem().string();
    const std::string extension = targetPath.extension().string();
    
    int counter = 1;
    std::filesystem::path uniquePath;
    
    do {
        std::ostringstream ss;
        ss << stem << "_" << std::setfill('0') << std::setw(3) << counter << extension;
        uniquePath = directory / ss.str();
        counter++;
    } while (std::filesystem::exists(uniquePath) && counter < 1000);
    
    if (counter >= 1000) {
        throw std::runtime_error("Could not generate unique filename after 1000 attempts");
    }
    
    return uniquePath;
} 