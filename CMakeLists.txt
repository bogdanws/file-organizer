cmake_minimum_required(VERSION 3.16)
project(FileOrganizer)

# Set C++ standard
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
# find_package(PkgConfig REQUIRED)  # Not needed for this project

# Include FetchContent for Google Test
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG v1.17.0
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add source directories
add_subdirectory(src)
add_subdirectory(tests)

# Enable testing
enable_testing() 