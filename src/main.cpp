#include "vire/ast/include.hpp"
#include "vire/lex/include.hpp"
#include "vire/parse/include.hpp"
#include "vire/errors/include.hpp"
#include "vire/proto/include.hpp"
#include "vire/v_analyzer/include.hpp"

#include <iostream>
#include <memory>

int main()
{
    auto file=vire::proto::openFile("res/test.ve");
    auto code=vire::proto::readFile(file, 1);

    //std::cout << code << std::endl;

    auto builder=std::make_unique<vire::errors::ErrorBuilder>("This program");
 
    auto lexer=std::make_unique<vire::Virelex>(code,0, builder);
    auto parser=std::make_unique<vire::Vireparse>(std::move(lexer)); 
    auto analyzer=std::make_unique<vire::VAnalyzer>(builder, code);

    auto ast=parser->ParseCode();

    if(!ast)
    {
        std::cout << "Error: Parsing failed" << std::endl;
        return 1;
    }
    else
    {
        std::cout << "Parsing successful" << std::endl;
    }

    bool success=analyzer->verifyCode(std::move(ast));
    if(success)
    {
        std::cout << "Verification successful" << std::endl;
    }
    else
    {
        std::cout << "Verification failed" << std::endl;
    }

    return 0;
}