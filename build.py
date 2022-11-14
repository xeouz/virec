#!/usr/bin/python3

import sys
import subprocess

commands={
    "cxx-cmake": "cmake . -DZLIB_LIBRARY=/usr/lib/x86_64-linux-gnu/libz.so -DCMAKE_PREFIX_PATH=/usr/local -GNinja -Bbuild",
    "wasm-cmake": "emcmake cmake ./wasm-lib -Wno-dev -G \"Unix Makefiles\" -DZLIB_LIBRARY=/home/dev0/Programming/emsdk/upstream/emscripten/cache/sysroot/lib/wasm32-emscripten/libz.a -DZLIB_INCLUDE_DIR=/usr/include/ -DLLVM_DIR=/home/dev0/Programming/llvm-project/build-wasm/lib/cmake/llvm -Bbuild",
    "cxx-build": "ninja -Cbuild -j8",
    "wasm-build": "make -Cbuild -j8",
    "cxx-run": "./VIRELANG",
}
build_types={
    "--release": "Release",
    "-r": "Release",

    "--debug": "Debug",
    "-d": "Debug",

    "--minsize": "MinSizeRel",
    "-minsz": "MinSizeRel",
}
build_arg="--debug"
run_executable=True
verbose=False

def run_command(command_list, run_verbose=False, cwd="./"):
    if run_verbose:
        return subprocess.run(command_list, cwd=cwd)

    return subprocess.run(command_list, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT, cwd=cwd)

def clean_cmake_cache():
    print("Cleaning CMakeCache...")
    subprocess.run(["rm", "./build/CMakeCache.txt", "-f"])
    subprocess.run(["rm", "-r", "./build/CMakeFiles", "-f"])

def build_cxx():
    ##########
    print("Building to native target..\n---")

    ##########
    clean_cmake_cache()
    
    ##########
    print("Running CMake command...")
    exec_command=commands['cxx-cmake'].split()
    exec_command.append("-DCMAKE_BUILD_TYPE="+build_types[build_arg])

    run_command(exec_command, run_verbose=verbose)

    ##########
    print("Executing build command...")
    run_command(commands["cxx-build"].split(), run_verbose=verbose)

    ##########
    if not run_executable:
        return

    print("Running executable")
    run_command(commands["cxx-run"], run_verbose=True, cwd="./build")

def build_wasm():
    ##########
    print("Building to WebAssembly..\n---")

    ##########
    clean_cmake_cache()
    
    ##########
    print("Running CMake command...")
    exec_command=commands['wasm-cmake'].split()

    run_command(exec_command, run_verbose=verbose)

    ##########
    print("Executing build command...")
    run_command(commands["wasm-build"].split(), run_verbose=verbose)    

def build_entry():
    if "--wasm" in sys.argv:
        build_wasm()
    else:
        build_cxx()

def main():
    global run_executable, verbose, build_arg

    for arg in sys.argv:
        if arg in build_types:
            build_arg=arg
            break

    if ("--no-run" in sys.argv) or ("-nr" in sys.argv):
        run_executable=False
    
    if ("--verbose" in sys.argv) or ("-v" in sys.argv):
        verbose=True

    build_entry()

if(__name__ == "__main__"):
    main()