#pragma once

#include <filesystem>
#include <memory>

#include "vire/proto/include.hpp"
#include "vire/v_compiler/include.hpp"

#include <emscripten/emscripten.h>
#include <emscripten/bind.h>

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
    bool compileSourceModule(std::string output_file_name="", bool write_to_file=true);

    void showErrors() const;
    errors::ErrorBuilder* const getErrorBuilder() const;
    VCompiler* const getCompiler() const;

    std::vector<unsigned char> const& getByteOutput();
    std::string const& getCompiledLLVMIR();
};

}