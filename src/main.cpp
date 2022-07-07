#include "vire/api/include.hpp"

#include <iostream>
#include <memory>

int main()
{
    auto file=vire::proto::openFile("res/test.ve");
    auto code=vire::proto::readFile(file, 1);

    auto builder=std::make_unique<vire::errors::ErrorBuilder>("This program");
 
    auto lexer=std::make_unique<vire::Virelex>(code,0, builder);
    auto parser=std::make_unique<vire::Vireparse>(std::move(lexer)); 
    auto analyzer=std::make_unique<vire::VAnalyzer>(builder.get(), code);

    auto ast=parser->ParseCode();

    std::cout << "Parsed Code" << std::endl;

    bool success=analyzer->verifyCode(std::move(ast));

    std::cout << "Analyzing Success: " << success << std::endl;

    if(!success)
    {
        return 1;
    }

    // Analysis and Frontend are done.
    // Now we can use the AST to generate the code.
    auto compiler=std::make_unique<vire::VCompiler>(std::move(analyzer));
    
    compiler->compileFunction("main");
    std::cout << "\n";

    return 0;
}