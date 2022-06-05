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

    auto ast=parser->ParseCode();
    std::cout << "Parsed AST" << std::endl;

    return 0;
}