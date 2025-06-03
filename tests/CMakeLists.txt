# Test files
set(TEST_SOURCES
    test_item_representation.cpp
    test_logger.cpp
    test_extension_condition.cpp
    test_configurable_rule.cpp
    test_configuration_parser.cpp
    test_rule_factory.cpp
    test_directory_organizer.cpp
    test_integration.cpp
)

# Create test executable
add_executable(file_organizer_tests ${TEST_SOURCES})

# Link with Google Test and our library
target_link_libraries(file_organizer_tests
    file_organizer_lib
    gtest_main
    gtest
)

# Include directories
target_include_directories(file_organizer_tests PRIVATE ${CMAKE_SOURCE_DIR}/src)

# Add tests to CTest
include(GoogleTest)
gtest_discover_tests(file_organizer_tests) 