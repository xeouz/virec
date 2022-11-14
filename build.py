#!/usr/bin/python3

import sys
import subprocess

from halo import Halo

class colors:
    HEADER = '\033[95m'
    OKBLUE = '\033[94m'
    OKCYAN = '\033[96m'
    OKGREEN = '\033[92m'
    OKPUR = '\u001b[35;1m'
    WARNING = '\033[93m'
    FAIL = '\033[91m'
    ENDC = '\033[0m'
    BOLD = '\033[1m'
    UNDERLINE = '\033[4m'
class options:
    verbose = False
    verbose_commands = True
    run_executable = True
    build_arg = "--debug"
commands = {
    "cxx-cmake": "cmake . -DCMAKE_PREFIX_PATH=/usr/local -GNinja -Bbuild",
    "wasm-cmake": "emcmake cmake ./wasm-lib -Wno-dev -GNinja -Bbuild -DZLIB_LIBRARY=/home/dev0/Programming/emsdk/upstream/emscripten/cache/sysroot/lib/wasm32-emscripten/libz.a -DZLIB_INCLUDE_DIR=/usr/include/ -DLLVM_DIR=/home/dev0/Programming/llvm-project/build-wasm/lib/cmake/llvm",
    "cxx-build": "ninja -Cbuild -j8",
    "wasm-build": "ninja -Cbuild -j8",
    "wasm-copy-wasm": "cp ./build/VIRELANG.wasm ./wasm-build/VIRELANG.wasm",
    "wasm-copy-js": "cp ./build/VIRELANG.js ./wasm-build/VIRELANG.js",
    "cxx-run": "./VIRELANG",
}
build_types = {
    "--release": "Release",
    "-r": "Release",

    "--debug": "Debug",
    "-d": "Debug",

    "--minsize": "MinSizeRel",
    "-minsz": "MinSizeRel",
}

def run_command(command_list, run_verbose=False, cwd="./"):
    if run_verbose:
        return subprocess.run(command_list, cwd=cwd)

    return subprocess.run(command_list, stdout=subprocess.DEVNULL, stderr=subprocess.STDOUT, cwd=cwd)

def clean_cmake_cache():
    subprocess.run(["rm", "./build/CMakeCache.txt", "-f"])
    subprocess.run(["rm", "-r", "./build/CMakeFiles", "-f"])
    print(f"{colors.OKGREEN}Cleaned CMakeCache{colors.ENDC}")

def build_cxx(opts):
    ##########
    print(f"{colors.OKBLUE}{colors.BOLD}Building to native target...{colors.ENDC}\n---")

    ##########
    clean_cmake_cache()
    
    ##########
    exec_command = commands["cxx-cmake"].split()
    exec_command.append("-DCMAKE_BUILD_TYPE="+build_types[opts.build_arg])

    cmd_text = "CMake command"
    if(opts.verbose_commands):
        cmd_text += ": \"" + commands["cxx-cmake"] + "\""

    with Halo(text=f"Executing {cmd_text}", spinner="arc", placement="right"):
        run_command(exec_command, run_verbose=opts.verbose)
    print(f"{colors.OKGREEN}Executed {cmd_text}{colors.ENDC}")

    ##########
    exec_command = commands["cxx-build"].split()

    cmd_text = "build command"
    if(opts.verbose_commands):
        cmd_text += ": \"" + commands["cxx-build"] + "\""

    with Halo(text=f"Executing {cmd_text}", spinner="arc", placement="right"):
        run_command(exec_command, run_verbose=opts.verbose)
    print(f"{colors.OKGREEN}Executed {cmd_text}{colors.ENDC}")
    print(f"{colors.OKGREEN}Build succeeded{colors.ENDC}")

    ##########
    if not opts.run_executable:
        return

    cmd_text=""
    if(opts.verbose_commands):
        cmd_text=": \"" + commands["cxx-run"] + "\""
    
    print(f"{colors.OKGREEN}Running executable{cmd_text}{colors.ENDC}")
    
    run_command(commands["cxx-run"].split(), run_verbose=True, cwd="./build")

def build_wasm(opts):
    ##########
    print(f"{colors.OKBLUE}{colors.BOLD}Building to native target...{colors.ENDC}\n---")

    ##########
    clean_cmake_cache()
    
    ##########
    exec_command = commands["wasm-cmake"].split()

    cmd_text = "CMake command"
    if(opts.verbose_commands):
        cmd_text += ": \"" + commands["wasm-cmake"] + "\""

    with Halo(text=f"Executing {cmd_text}", spinner="arc", placement="right"):
        run_command(exec_command, run_verbose=opts.verbose)
    print(f"{colors.OKGREEN}Executed {cmd_text}{colors.ENDC}")

    ##########
    exec_command = commands["wasm-build"].split()

    cmd_text = "build command"
    if(opts.verbose_commands):
        cmd_text += ": \"" + commands["wasm-build"] + "\""

    with Halo(text=f"Executing {cmd_text}", spinner="arc", placement="right"):
        run_command(exec_command, run_verbose=opts.verbose)
    print(f"{colors.OKGREEN}Executed {cmd_text}{colors.ENDC}")
    print(f"{colors.OKGREEN}Build succeeded{colors.ENDC}")

def build_entry(opts):
    if "--wasm" in sys.argv:
        build_wasm(opts)
    else:
        build_cxx(opts)

def main():
    opts = options()

    for arg in sys.argv:
        if arg in build_types:
            opts.build_arg=arg
            break

    if ("--compile" in sys.argv) or ("-c" in sys.argv):
        opts.run_executable=False
    
    if ("--verbose-all" in sys.argv) or ("-va" in sys.argv):
        opts.verbose = True
        opts.verbose_commands= True
    if ("--verbose" in sys.argv) or ("-v" in sys.argv):
        opts.verbose = False
        opts.verbose_commands = True
    if ("--silent" in sys.argv) or ("-s" in sys.argv):
        opts.verbose = False
        opts.verbose_commands = False

    build_entry(opts)

if(__name__ == "__main__"):
    main()