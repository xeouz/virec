#include "vire/includes.hpp"

#include <iostream>
#include <memory> 

/*
int main(int argc, char ** argv)
{
    std::string filename="output";
    auto file=vire::proto::openFile("res/test.ve"); 
    auto code=vire::proto::readFile(file, 1);

    auto builder=std::make_unique<vire::errors::ErrorBuilder>("This program");
 
    auto lexer=std::make_unique<vire::VLexer>(code, 0, builder.get());
    auto parser=std::make_unique<vire::VParser>(std::move(lexer)); 
    auto analyzer=std::make_unique<vire::VAnalyzer>(builder.get(), code);

    auto ast=parser->ParseSourceModule();
 
    // std::cout << "Parsed Code" << std::endl;

    bool success=analyzer->verifySourceModule(std::move(ast));

    std::cout << "Analyzer Success: " << success << std::endl;

    if(!success) return 1;

    // std::cout << "\n\n";

    // Analysis and Frontend are done.
    // Now we can use the AST to generate the code.
    
    // std::cout << "Generating Code" << std::endl; 
    auto compiler=std::make_unique<vire::VCompiler>(std::move(analyzer), filename);

    compiler->compileModule();

    llvm::verifyModule(*compiler->getModule());

    std::cout << "Code Generated: \n" << std::endl;
    std::cout << compiler->getCompiledOutput() << std::endl;
    //std::cout << "---\n" << std::endl;

    compiler->compileToObjectFile(filename+".wasm", "wasm32-unknown-unknown-wasm");

    return 0;
}
*/

int main()
{
    auto api=vire::VApi::loadFromFile("res/test.ve", "wasm32");

    api->parseSourceModule();

    bool s=api->verifySourceModule();
    if(!s) return 1;
    
    s=api->compileSourceModule("test.wasm", false);
    api->getErrorBuilder()->showErrors();
    if(!s) return 1;

    std::cout << "Compiled file to test.wasm" << std::endl;
    std::cout << api->getCompiler()->getCompiledOutput() << std::endl;

    return 1;
} 