#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

namespace vire
{

class IdentifierExprAST : public ExprAST
{
    std::string name;
public:
    IdentifierExprAST(std::unique_ptr<Viretoken> name, int asttype=ast_var) : name(name->value), ExprAST("", asttype)
    {
        setToken(std::move(name));
    }

    virtual std::string const& getName() const
    {
        return name;
    }
    virtual Viretoken* const getToken() const
    {
        return token.get();
    }
};

// VariableExprAST - Class for referencing a variable, eg - `myvar`
class VariableExprAST : public IdentifierExprAST
{
    std::string name;
public:
    VariableExprAST(std::unique_ptr<Viretoken> name) : 
    name(name->value), IdentifierExprAST(std::move(name))
    {
    } 
};

}