#include "vire/api/include.hpp"

#include <iostream>
#include <memory> 

int main(int argc, char ** argv)
{
    auto file=vire::proto::openFile("res/test.ve");
    auto code=vire::proto::readFile(file, 1);

    auto builder=std::make_unique<vire::errors::ErrorBuilder>("This program");
 
    auto lexer=std::make_unique<vire::Virelex>(code, 0, builder);
    auto parser=std::make_unique<vire::Vireparse>(std::move(lexer)); 
    auto analyzer=std::make_unique<vire::VAnalyzer>(builder.get(), code);

    auto ast=parser->ParseCode();
 
    std::cout << "Parsed Code" << std::endl;

    bool success=analyzer->verifyCode(std::move(ast));

    std::cout << "Analyzing Success: " << success << std::endl;

    if(!success) return success;

    std::cout << "\n\n";

    // Analysis and Frontend are done.
    // Now we can use the AST to generate the code.
    
    std::cout << "Generating Code" << std::endl; 
    auto compiler=std::make_unique<vire::VCompiler>(std::move(analyzer), "test");

    compiler->compileExtern("put");
    compiler->compileFunction("main");  

    std::cout << "Compiling Success\n---\n" << std::endl;
    std::cout << compiler->getCompiledOutput() << std::endl;
    std::cout << "---\n" << std::endl;

    compiler->compileToObjectFile("test.o"); 
    
    std::cout << "\n";

    return 1;
}