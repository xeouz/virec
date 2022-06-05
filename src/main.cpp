#include "Vire/AST-Parse/Include.hpp"
#include "Vire/Proto/Include.hpp"
#include "Vire/VAnalyzer/Include.hpp"
#include "Vire/Error-Builder/Include.hpp"

int main()
{
    auto file=vire::proto::openFile("resources/test.ve");
    auto code=vire::proto::readFile(file, 1);

    //std::cout << code << std::endl;

    auto lexer=std::make_unique<vire::Virelex>(code);
    auto parser=std::make_unique<vire::Vireparse>(std::move(lexer));

    parser->getNextToken();
    auto ast=vire::case_static<vire::VariableDefAST>(std::move(parser->ParseVariableDef()));

    auto analyzer=std::make_unique<vire::VAnalyzer>();
    analyzer->verifyVarDef(ast);

    auto builder=std::make_unique<vire::errors::ErrorBuilder>();
    builder->setPrefix("This program");
    auto f=builder->constructCodePosition(code, 2,5,4);

    return 0;
}