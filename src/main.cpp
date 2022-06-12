#include "Vire/AST-Parse/Include.hpp"
#include "Vire/Proto/Include.hpp"
#include "Vire/VAnalyzer/Include.hpp"
#include "Vire/Error-Builder/Include.hpp"

int main()
{
    auto file=vire::proto::openFile("resources/test.ve");
    auto code=vire::proto::readFile(file, 1);

    //std::cout << code << std::endl;

    auto builder=std::make_unique<vire::errors::ErrorBuilder>("This program");

    auto lexer=std::make_unique<vire::Virelex>(code,0, builder);
    auto parser=std::make_unique<vire::Vireparse>(std::move(lexer)); 
  
    parser->getNextToken();
    auto ast=std::move(parser->ParseForExpr());
 
    auto analyzer=std::make_unique<vire::VAnalyzer>(builder, code);
    std::cout << analyzer->verifyExpr(ast) << std::endl;

    builder->showErrors();

    return 0;
}