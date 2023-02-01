#pragma once

#include <filesystem>
#include <memory>

#include "vire/proto/include.hpp"
#include "vire/v_compiler/include.hpp"

#ifdef VIRE_USE_EMCC
#include <emscripten/emscripten.h>
#include <emscripten/bind.h>
#endif

namespace vire
{

class VApi
{
    std::unique_ptr<VParser> parser;
    std::unique_ptr<VCompiler> compiler;
    std::unique_ptr<ModuleAST> ast;
    std::unique_ptr<errors::ErrorBuilder> ebuilder;

    std::string source_code;
    std::string target;

    std::vector<unsigned char> byte_output;
private:
    void internal_setup();

public:
    VApi(std::unique_ptr<VParser> parser, std::unique_ptr<VCompiler> compiler, 
    std::unique_ptr<errors::ErrorBuilder> ebuilder, std::string source_code="", std::string target="sys");
    VApi();

    static std::unique_ptr<VApi> loadFromFile(std::string input_file_path, std::string compilation_target="sys");
    static std::unique_ptr<VApi> loadFromText(std::string input_code, std::string compilation_target="sys");

    bool parseSourceModule();
    bool verifySourceModule();
    bool compileSourceModule(std::string const& output_file_name="", bool write_to_file=true, Optimization opt_level=Optimization::O0, bool enable_lto=false);
    bool compileSourceModuleStringOpt(std::string const& output_file_name="", bool write_to_file=true, std::string const& opt_level="O0", bool enable_lto=false);

    void setSourceCode(std::string new_code);
    void reset();

    void showErrors() const;
    errors::ErrorBuilder* const getErrorBuilder() const;
    VCompiler* const getCompiler() const;

    std::vector<unsigned char> const& getByteOutput();
    std::string const& getCompiledLLVMIR();
};

}