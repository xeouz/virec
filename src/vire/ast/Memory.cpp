#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <vector>
#include <memory>

namespace vire
{

class UnsafeExprAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> body;
public:
    UnsafeExprAST(std::vector<std::unique_ptr<ExprAST>> body) : body(std::move(body)), ExprAST("",ast_unsafe)
    {}

    std::vector<std::unique_ptr<ExprAST>> const& getBody() {return body;}
};

class ReferenceExprAST : public ExprAST
{
    std::unique_ptr<ExprAST> var;
public:
    ReferenceExprAST(std::unique_ptr<ExprAST> var) : var(std::move(var)), ExprAST("",ast_reference)
    {}

    ExprAST* const getVar() { return var.get(); }
};

}