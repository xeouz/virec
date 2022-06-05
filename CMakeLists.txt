# Configuration setup
cmake_minimum_required(VERSION 3.16.3)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

project(VIRELANG VERSION 3.5.1)

# Add main executable
add_executable(VIRELANG src/main.cpp)

# Compiling Parser lib
include(cmake/Parser.cmake)
include(cmake/Proto.cmake)
include(cmake/VAnalyzer.cmake)
include(cmake/ErrorBuilder.cmake)

# Copy the resources to the build directory
add_custom_command(
    TARGET VIRELANG PRE_BUILD
    COMMAND $(CMAKE_COMMAND) -E copy_directory
        $(CMAKE_SOURCE_DIR)/src/resources $<TARGET_FILE_DIR:VIRELANG>/resources
)

# Run the output
add_custom_command(
    TARGET VIRELANG POST_BUILD
    COMMAND ./VIRELANG
)