#include "Vire/AST-Parse/Include.hpp"
#include "Vire/Proto/Include.hpp"

int main()
{
    auto file=vire::proto::openFile("resources/test.ve");
    auto code=vire::proto::readFile(file, 1);

    auto lexer=std::make_unique<vire::Virelex>(code);
    auto parser=std::make_unique<vire::Vireparse>(std::move(lexer));

    parser->getNextToken();
}