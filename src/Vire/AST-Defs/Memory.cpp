#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <vector>
#include <memory>

namespace vire
{

class UnsafeAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> body;
public:
    UnsafeAST(std::vector<std::unique_ptr<ExprAST>> body) : body(std::move(body)), ExprAST("",ast_unsafe)
    {}

    std::vector<std::unique_ptr<ExprAST>> getBody() {return std::move(body);}
};

class ReferenceExprAST : public ExprAST
{
    std::unique_ptr<ExprAST> var;
public:
    ReferenceExprAST(std::unique_ptr<ExprAST> var) : var(std::move(var)), ExprAST("",ast_reference)
    {}

    std::unique_ptr<ExprAST> getVar() {return std::move(var);}
};

}