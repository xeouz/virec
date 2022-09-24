#include "vire/api/include.hpp"

#include <iostream>
#include <memory> 

int main(int argc, char ** argv)
{
    std::string filename="output";
    auto file=vire::proto::openFile("res/test.ve");
    auto code=vire::proto::readFile(file, 1);

    auto builder=std::make_unique<vire::errors::ErrorBuilder>("This program");
 
    auto lexer=std::make_unique<vire::Virelex>(code, 0, builder);
    auto parser=std::make_unique<vire::Vireparse>(std::move(lexer)); 
    auto analyzer=std::make_unique<vire::VAnalyzer>(builder.get(), code);

    auto ast=parser->ParseCode();
 
    // std::cout << "Parsed Code" << std::endl;

    bool success=analyzer->verifyCode(std::move(ast));

    std::cout << "Analyzer Success: " << success << std::endl;

    if(!success) return 1;

    // std::cout << "\n\n";

    // Analysis and Frontend are done.
    // Now we can use the AST to generate the code.
    
    // std::cout << "Generating Code" << std::endl; 
    auto compiler=std::make_unique<vire::VCompiler>(std::move(analyzer), filename);

    compiler->compileExtern("puti");
    compiler->compileExtern("putd");
    compiler->compileExtern("putb");
    compiler->compileExtern("put");
    compiler->compileFunction("test");
    compiler->compileFunction("main");

    //std::cout << "Code Generated: \n" << std::endl;
    // std::cout << "---\n" << std::endl;

    compiler->compileToObjectFile(filename+".o");
    
    // std::cout << "\n";
    std::cout << "Compiled " << filename << ", Output: \n" << std::endl;
    
    std::string command="clang test.cpp ";
    command.append(filename+".o"); 
    command.append(" -o ");
    command.append(filename);
 
    system(command.c_str());
    
    command="./";
    command.append(filename);

    system(command.c_str());

    return 0;
}