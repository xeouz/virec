#include <iostream>

#include "Vire/Proto/File.cpp"

#include "Vire/Lex/Include.hpp"
#include "Vire/AST-Defs/Include.hpp"
#include "Vire/AST-Parse/Include.hpp"
#include "Vire/VCompiler/Include.hpp"

int main()
{
    auto file=vire::proto::openFile("test.ve");
    auto code=vire::proto::readFile(file,1);

    // std::cout << code << "\n" << std::endl;

    auto lex=std::make_unique<vire::Virelex>(code);
    auto parser=std::make_unique<vire::Vireparse>(std::move(lex));
    
    parser->getNextToken();
    parser->ParsePrimary();
}