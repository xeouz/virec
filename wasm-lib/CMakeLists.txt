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

get_filename_component(SRC_DIR ../ ABSOLUTE)
message(${SRC_DIR})
set(CXX_COMPILE_FLAGS "-I${SRC_DIR}/src -lembind")
set(CXX_COMPILE_FLAGS "${CXX_COMPILE_FLAGS} -s MODULARIZE -s WASM=1 -s EXPORT_ES6 -s ENVIRONMENT=web -s EXPORT_NAME=\"createGlobalModule\"")
set(CXX_COMPILE_FLAGS "${CXX_COMPILE_FLAGS} --pre-js ${SRC_DIR}/wasm-lib/emcc-pre.js --extern-pre-js ${SRC_DIR}/wasm-lib/emcc-extern-pre.js")
set(CXX_COMPILE_FLAGS "${CXX_COMPILE_FLAGS} -s -Wno-unused-command-line-argument -sERROR_ON_UNDEFINED_SYMBOLS=0 -sNO_DISABLE_EXCEPTION_CATCHING")
#set(CXX_COMPILE_FLAGS "${CXX_COMPILE_FLAGS} --closure 1 --closure-args=--compilation_level=SIMPLE_OPTIMIZATIONS -Oz -sFILESYSTEM=0")
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${CXX_COMPILE_FLAGS}")

# -- LLVM
include_directories(${LLVM_INCLUDE_DIRS})
separate_arguments(LLVM_DEFINITIONS_LIST NATIVE_COMMAND ${LLVM_DEFINITIONS})
add_definitions(${LLVM_DEFINITIONS_LIST})

# -- Add main executable
add_compile_definitions(VIRE_USE_EMCC)
add_executable(VIRELANG ${SRC_DIR}/src/main.cpp)

# -- LLVM Libraries
link_libraries()
execute_process(COMMAND llvm-config --libs WebAssembly OUTPUT_VARIABLE LIBS)
execute_process(COMMAND llvm-config --system-libs OUTPUT_VARIABLE SYS_LIBS)
execute_process(COMMAND llvm-config --ldflags OUTPUT_VARIABLE LDF)

string(STRIP ${LIBS} LIBS)
string(STRIP ${SYS_LIBS} SYS_LIBS)
string(STRIP ${LDF} LDF)

link_libraries(${LIBS} ${SYS_LIBS} -L/home/dev0/Programming/llvm-project/build-wasm/lib)

set(VIRE_SRC_PATH "${SRC_DIR}/src/vire")

# -- Compiling Parser, Proto, Analyzer, ErrorBuilder libs
include(${VIRE_SRC_PATH}/api/VApi.cmake)
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
        ${SRC_DIR}/src/res $<TARGET_FILE_DIR:VIRELANG>/res
)