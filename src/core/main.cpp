#include "Logger.h"
#include "ConfigurationParser.h"
#include "RuleFactory.h"
#include "DirectoryOrganizer.h"
#include <iostream>
#include <filesystem>

int main(int argc, char* argv[]) {
    // default configuration file path
    std::string configFilePath = "sorter_config.txt";
    
    // parse command line arguments
    if (argc > 1) {
        configFilePath = argv[1];
    }
    
    // check if config file exists
    if (!std::filesystem::exists(configFilePath)) {
        std::cerr << "Configuration file not found: " << configFilePath << std::endl;
        std::cerr << "Usage: " << argv[0] << " [config_file_path]" << std::endl;
        return 1;
    }
    
    try {
        // parse configuration
        ConfigurationParser parser;
        if (!parser.parseFile(configFilePath)) {
            std::cerr << "Failed to parse configuration file:" << std::endl;
            for (const auto& error : parser.getErrors()) {
                std::cerr << "  - " << error << std::endl;
            }
            return 1;
        }
        
        const auto&[sourceDir, targetBaseDir, dryRun, logLevel, logFile] = parser.getGlobalConfig();
        
        // initialize logger with configuration settings
        Logger::instance().init(logLevel, logFile);
        Logger::instance().info("File Organizer starting...");
        Logger::instance().info("Configuration loaded from: " + configFilePath);
        
        // create rules using factory
        RuleFactory factory;
        auto rules = factory.createRulesFromConfig(parser);
        
        if (rules.empty()) {
            Logger::instance().warning("No valid rules found in configuration");
            return 1;
        }
        
        Logger::instance().info("Loaded " + std::to_string(rules.size()) + " rules");
        
        // create and run directory organizer
        DirectoryOrganizer organizer(
            sourceDir,
            targetBaseDir,
            std::move(rules),
            dryRun
        );
        
        organizer.scanAndOrganize();
        
        // display final statistics
        const auto&[filesProcessed, filesMovedOrWouldMove, filesSkipped, directoriesProcessed, directoriesMovedOrWouldMove, directoriesSkipped, errors] = organizer.getStatistics();
        Logger::instance().info("=== Final Statistics ===");
        Logger::instance().info("Files processed: " + std::to_string(filesProcessed));
        Logger::instance().info("Files moved: " + std::to_string(filesMovedOrWouldMove));
        Logger::instance().info("Files skipped: " + std::to_string(filesSkipped));
        Logger::instance().info("Directories processed: " + std::to_string(directoriesProcessed));
        Logger::instance().info("Directories moved: " + std::to_string(directoriesMovedOrWouldMove));
        Logger::instance().info("Directories skipped: " + std::to_string(directoriesSkipped));
        Logger::instance().info("Errors: " + std::to_string(errors));
        
        if (dryRun) {
            Logger::instance().info("DRY RUN MODE: No files were actually moved");
        }
        
        Logger::instance().info("File Organizer completed successfully.");
        
        return errors > 0 ? 1 : 0;
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << std::endl;
        return 1;
    }
}

