# -- Configuration setup
cmake_minimum_required(VERSION 3.16.3)

project(VIRELANG VERSION 3.5.1)

find_package(ZLIB REQUIRED)
find_package(LLVM REQUIRED CONFIG)
message(STATUS "Found LLVM: ${LLVM_VERSION}")
message(STATUS "Using LLVMConfig.cmake in ${LLVM_DIR}")

set(CMAKE_BUILD_TYPE Debug)

# -- Compile Flags
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED true)

message(STATUS CMAKE_PREFIX_PATH)

set(CXX_COMPILE_FLAGS "-I${CMAKE_SOURCE_DIR}/src")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_COMPILE_FLAGS}")

# -- LLVM
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# -- Add main executable
add_executable(VIRELANG src/main.cpp)

# -- LLVM Libraries
foreach(target ${LLVM_TARGETS_TO_BUILD})
    list(APPEND targets "LLVM${target}CodeGen")
endforeach()
llvm_map_components_to_libnames(llvm_libs ${LLVM_TARGETS_TO_BUILD} support core irreader)
target_link_libraries(VIRELANG PRIVATE ${llvm_libs})

set(VIRE_SRC_PATH "${CMAKE_CURRENT_SOURCE_DIR}/src/vire")

# -- Compiling Parser, Proto, Analyzer, ErrorBuilder libs
include(${VIRE_SRC_PATH}/parse/Parser.cmake)
include(${VIRE_SRC_PATH}/proto/Proto.cmake)
include(${VIRE_SRC_PATH}/v_analyzer/VAnalyzer.cmake)
include(${VIRE_SRC_PATH}/errors/ErrorBuilder.cmake)
include(${VIRE_SRC_PATH}/config/Config.cmake)
include(${VIRE_SRC_PATH}/v_compiler/VCompiler.cmake)

# -- Copy the resources to the build directory
add_custom_command(
    TARGET VIRELANG POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_SOURCE_DIR}/src/res $<TARGET_FILE_DIR:VIRELANG>/res
)

# -- Run the program
add_custom_command(
    TARGET VIRELANG POST_BUILD
    COMMAND valgrind ./VIRELANG
)