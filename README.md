# File Organizer

A configurable C++ application for automatically organizing files and directories based on user-defined rules. The application reads rules from a configuration file and moves items from a source directory to organized target directories according to file properties like extension, size, age, and name patterns.

## Features

- **Rule-Based Organization**: Define custom rules to organize files based on various criteria
- **Flexible Configuration**: External text-based configuration file for easy customization
- **Dry Run Mode**: Preview changes without actually moving files
- **Comprehensive Logging**: Detailed logging with configurable levels and optional file output
- **Extensible Architecture**: Easy to add new condition types and rules
- **Priority-Based Rules**: Rules are applied in order of priority
- **File and Directory Support**: Can organize both individual files and entire directories
- **Error Handling**: Graceful handling of file system errors and edge cases

## Supported Conditions

- **Extension Matching**: Organize files by their file extension (`.pdf`, `.jpg`, etc.)
- **Size Conditions**: Filter by file size with units (KB, MB, GB)
- **Age Conditions**: Filter by file modification date with time units (d, m, y)
- **Name Matching**: Pattern-based name matching with wildcards (coming soon)
- **Empty Directory Detection**: Identify empty directories (coming soon)

## Build Requirements

- C++23 or later
- CMake 3.16 or later
- Standard C++ filesystem library support
- Google Test (for running tests, optional)

## Building the Project

1. Clone the repository:
```bash
git clone <repository-url>
cd file-organizer
```

2. Create a build directory and compile:
```bash
mkdir build
cd build
cmake ..
make
```

3. (Optional) Run tests:
```bash
make test
```

## Usage

### Basic Usage

```bash
./build/src/file_organizer [config_file_path]
```

If no configuration file is specified, the application will look for `sorter_config.txt` in the current directory.

### Example Configuration File

Create a configuration file (e.g., `my_config.txt`) with your organization rules:

```ini
# Source directory to organize
SOURCE_DIR: /path/to/messy_folder

# Base directory where sorted items will be moved
TARGET_BASE_DIR: /path/to/organized_folder

# Optional: Dry Run Mode (true/false, default: false)
DRY_RUN: true

# Optional: Logging Configuration
LOG_LEVEL: INFO
LOG_FILE: organizer.log

# Rule Definitions
RULE:
  TARGET_PATH: documents/pdfs
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: images/photos
  PRIORITY: 20
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .jpg
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: archives
  PRIORITY: 30
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .zip
  END_CONDITIONS
END_RULE

# Catch-all rule for unmatched items
RULE:
  TARGET_PATH: others
  PRIORITY: 1000
  APPLIES_TO: any
  CONDITIONS:
  END_CONDITIONS
END_RULE
```

### Running the Organizer

```bash
# Run with default config file
./build/src/file_organizer

# Run with custom config file
./build/src/file_organizer my_config.txt
```

## Configuration File Format

### Global Settings

- `SOURCE_DIR`: Path to the directory containing files to organize
- `TARGET_BASE_DIR`: Base path where organized files will be moved
- `DRY_RUN`: Set to `true` to preview changes without moving files
- `LOG_LEVEL`: Logging verbosity (`DEBUG`, `INFO`, `WARNING`, `ERROR`)
- `LOG_FILE`: Optional path to log file (logs to console if not specified)

### Rule Structure

```ini
RULE:
  TARGET_PATH: relative/path/from/target_base
  PRIORITY: number (lower = higher priority)
  APPLIES_TO: file|folder|any
  CONDITIONS:
    CONDITION_TYPE: value
    # Multiple conditions are combined with AND logic
  END_CONDITIONS
END_RULE
```

### Available Conditions

#### Extension Matching
```ini
EXTENSION: .pdf
```
Matches files with the specified extension (case-insensitive).

#### Size Conditions
```ini
SIZE_GREATER_THAN: 100MB
SIZE_LESS_THAN: 10KB
```
Filter files by size. Supports units: B (bytes), KB, MB, GB, TB.

#### Age Conditions
```ini
AGE_OLDER_THAN: 30d
AGE_NEWER_THAN: 7d
```
Filter files by modification date. Supports units: d (days), m (months), y (years).

#### Future Conditions (Planned)
- `NAME_MATCHES: *backup*` - Files matching name pattern with wildcards
- `IS_EMPTY: true` - Empty directories

## Examples

### Organizing a Download Folder

```ini
SOURCE_DIR: /home/user/Downloads
TARGET_BASE_DIR: /home/user/Organized

DRY_RUN: false
LOG_LEVEL: INFO

RULE:
  TARGET_PATH: documents
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .pdf
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: images
  PRIORITY: 20
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .jpg
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: software
  PRIORITY: 30
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .exe
  END_CONDITIONS
END_RULE
```

### Photo Organization

```ini
SOURCE_DIR: /home/user/Pictures/Unsorted
TARGET_BASE_DIR: /home/user/Pictures/Organized

RULE:
  TARGET_PATH: photos/jpeg
  PRIORITY: 10
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .jpg
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: photos/png
  PRIORITY: 20
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .png
  END_CONDITIONS
END_RULE

RULE:
  TARGET_PATH: photos/raw
  PRIORITY: 30
  APPLIES_TO: file
  CONDITIONS:
    EXTENSION: .raw
  END_CONDITIONS
END_RULE
```

## Logging

The application provides detailed logging at multiple levels:

- **DEBUG**: Detailed information about rule evaluation and file processing
- **INFO**: General information about operations performed
- **WARNING**: Non-critical issues (e.g., files that don't match any rule)
- **ERROR**: Critical errors that prevent normal operation

Example log output:
```
[INFO] File Organizer starting...
[INFO] Configuration loaded from: config.txt
[INFO] Loaded 4 rules
[INFO] Starting file organization process...
[INFO] Processing file: /source/document.pdf
[DEBUG] Evaluating rule: PDF Documents (priority: 10)
[DEBUG] Extension condition: .pdf matches .pdf
[INFO] Moving file: /source/document.pdf -> /target/documents/pdfs/document.pdf
[INFO] Files processed: 1, Files moved: 1, Errors: 0
```

## Architecture

The application follows SOLID principles and implements several design patterns:

- **Strategy Pattern**: Different sorting rules implement the `ISortingRule` interface
- **Factory Pattern**: `RuleFactory` creates rule and condition objects from configuration
- **Singleton Pattern**: `Logger` class provides centralized logging
- **Composite Pattern**: Rules can contain multiple conditions

### Key Components

- `ItemRepresentation`: Represents file system items with their properties
- `ICondition`: Interface for rule conditions
- `ISortingRule`: Interface for sorting rules
- `ConfigurableRule`: Implementation that combines multiple conditions
- `RuleFactory`: Creates rules and conditions from configuration
- `ConfigurationParser`: Parses configuration files
- `DirectoryOrganizer`: Main orchestrator for the organization process
- `Logger`: Centralized logging system

## Testing

The project includes comprehensive unit and integration tests using Google Test:

```bash
# From the build directory, run all tests
cd build
ctest

# Or run the test executable directly
./tests/file_organizer_tests

# Run tests with verbose output
./tests/file_organizer_tests --gtest_list_tests

# Run specific test suites
./tests/file_organizer_tests --gtest_filter="LoggerTest*"
./tests/file_organizer_tests --gtest_filter="IntegrationTest*"
```

## Error Handling

The application handles various error conditions gracefully:

- Invalid or missing configuration files
- Inaccessible source or target directories
- File permission issues
- Disk space problems
- Invalid rule configurations

## Contributing

1. Fork the repository
2. Create a feature branch
3. Add tests for new functionality
4. Ensure all tests pass
5. Submit a pull request

## License

[Add your license information here]

## Troubleshooting

### Common Issues

**"Configuration file not found"**
- Ensure the config file path is correct
- Check file permissions

**"Failed to parse configuration file"**
- Verify the configuration file syntax
- Check for missing END_CONDITIONS or END_RULE tags

**Files not being moved**
- Check if DRY_RUN is set to true
- Verify source and target directory permissions
- Review rule priorities and conditions

**No rules matched**
- Add a catch-all rule with high priority number
- Check condition syntax in configuration file

### Getting Help

For issues and questions:
1. Check the log output for detailed error messages
2. Try running in dry mode first to preview changes
3. Verify your configuration file syntax against the examples
4. Review the project documentation in `PLAN.md` and `PROJECT_PROGRESS.md`
