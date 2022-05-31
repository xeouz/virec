#pragma once

#include "../Lex/Include.hpp"
#include "../AST-Defs/ASTType.hpp"
#include "../AST-Parse/Include.hpp"

namespace vire
{
class VCompiler
{
    Vireparse* parser;
public:
    VCompiler(Vireparse* parser) : parser(parser) {}
};
}