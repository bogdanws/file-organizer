# Source files
set(SOURCES
    core/Logger.cpp
    core/ConfigurationParser.cpp
    core/RuleFactory.cpp
    core/DirectoryOrganizer.cpp
    rules/ConfigurableRule.cpp
    conditions/ExtensionCondition.cpp
    models/ItemRepresentation.cpp
)


# Header files
set(HEADERS
    core/Logger.h
    core/ConfigurationParser.h
    core/RuleFactory.h
    core/DirectoryOrganizer.h
    rules/ConfigurableRule.h
    rules/ISortingRule.h
    rules/ICondition.h
    conditions/ExtensionCondition.h
    models/ItemRepresentation.h
)

# Create library
add_library(file_organizer_lib ${SOURCES} ${HEADERS})

# Include directories
target_include_directories(file_organizer_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Link filesystem library (conditional for different compilers)
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS "9.0")
    target_link_libraries(file_organizer_lib stdc++fs)
endif()

# Add main executable
add_executable(file_organizer core/main.cpp)
target_link_libraries(file_organizer file_organizer_lib)