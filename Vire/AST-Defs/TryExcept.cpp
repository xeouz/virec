#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>

namespace vire
{

class TryExprAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> body;
    std::unique_ptr<ExprAST> CatchException;
    std::vector<std::unique_ptr<ExprAST>> CatchBody;
public:
    char has_except;
    TryExprAST(std::vector<std::unique_ptr<ExprAST>> body) : body(std::move(body)), ExprAST("",ast_try)
    , has_except(0) {}
    TryExprAST(std::vector<std::unique_ptr<ExprAST>> body, 
    std::unique_ptr<ExprAST> CatchException, std::vector<std::unique_ptr<ExprAST>> CatchBody)
    : body(std::move(body)), CatchException(std::move(CatchException)), CatchBody(std::move(CatchBody))
    , ExprAST("",ast_try), has_except(1) {}

    std::unique_ptr<ExprAST> getCatchException() {return std::move(CatchException);}
    std::vector<std::unique_ptr<ExprAST>> getTryBody() {return std::move(body);}
    std::vector<std::unique_ptr<ExprAST>> getCatchBody() {return std::move(CatchBody);}
};

}