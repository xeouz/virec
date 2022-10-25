#pragma once

#include <filesystem>

#include "vire/proto/include.hpp"
#include "vire/v_compiler/include.hpp"

namespace vire
{

class VApi
{
    std::unique_ptr<VParser> parser;
    std::unique_ptr<VCompiler> compiler;
    std::unique_ptr<ModuleAST> ast;
    std::unique_ptr<errors::ErrorBuilder> ebuilder;

    std::string inp_file_path;
    std::string out_file_name;
    std::string target;
public:
    VApi(std::string target, std::string input_file_path, std::string output_file_name="")
    : target(target), inp_file_path(input_file_path), out_file_name(output_file_name)
    {
        if(out_file_name=="")
        {
            auto current_path=std::filesystem::current_path().remove_filename().string();
            
        #ifdef  _WIN32
            out_file_name=current_path+"/a.exe";
        #endif
        #ifndef _WIN32
            out_file_name=current_path+"/a.out";
        #endif
        }

        auto file=vire::proto::openFile(inp_file_path);
        std::string src=vire::proto::readFile(file, 1);

        ebuilder=std::make_unique<errors::ErrorBuilder>("This program");

        auto lexer=std::make_unique<VLexer>(src, false, ebuilder.get());
        parser=std::make_unique<VParser>(std::move(lexer));

        auto analyzer=std::make_unique<VAnalyzer>(ebuilder.get(), src);
        compiler=std::make_unique<VCompiler>(std::move(analyzer), out_file_name);
    }

    bool parseSourceModule();
    bool verifySourceModule();
    bool compileSourceModule();
    
    errors::ErrorBuilder* const getErrorBuilder() const;
    VCompiler* const getCompiler() const;
};

}