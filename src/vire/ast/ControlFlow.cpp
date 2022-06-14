#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>

namespace vire
{

class IfThenExpr
{
    std::unique_ptr<ExprAST> Condition;
    std::unique_ptr<ExprAST> ThenBlock;
public:
    IfThenExpr(std::unique_ptr<ExprAST> Condition, std::unique_ptr<ExprAST> ThenBlock)
    : Condition(std::move(Condition)), ThenBlock(std::move(ThenBlock))
    {}

    const std::unique_ptr<ExprAST>& getCondition() {return Condition;}
    const std::unique_ptr<ExprAST>& getThenBlock() {return ThenBlock;}
};

class IfExprAST : public ExprAST
{
    std::unique_ptr<IfThenExpr> IfThen;
    std::unique_ptr<ExprAST> ElseBlock;
    std::vector<std::unique_ptr<IfThenExpr>> ElifLadder;
    std::unique_ptr<ExprAST> FinallyBlock;
public:
    char has_else, has_elif, has_finally;

    IfExprAST(std::unique_ptr<IfThenExpr> IfThen) : IfThen(std::move(IfThen)), has_else(0), has_finally(0),
    has_elif(0), ExprAST("",ast_if) {}
    IfExprAST(std::unique_ptr<IfThenExpr> IfThen, std::unique_ptr<ExprAST> ElseBlock) 
    : IfThen(std::move(IfThen)), ElseBlock(std::move(ElseBlock)), has_else(1), has_finally(0), has_elif(0),
    ExprAST("", ast_if) {}
    IfExprAST(std::unique_ptr<IfThenExpr> IfThen, std::unique_ptr<ExprAST> ElseBlock,
    std::vector<std::unique_ptr<IfThenExpr>> ElifLadder)
    : IfThen(std::move(IfThen)), ElseBlock(std::move(ElseBlock)), ElifLadder(std::move(ElifLadder)),
    has_else(1), has_finally(0), has_elif(1), ExprAST("",ast_if) {}
    IfExprAST(std::unique_ptr<IfThenExpr> IfThen, std::unique_ptr<ExprAST> ElseBlock,
    std::vector<std::unique_ptr<IfThenExpr>> ElifLadder, std::unique_ptr<ExprAST> FinallyBlock)
    : IfThen(std::move(IfThen)), ElseBlock(std::move(ElseBlock)), ElifLadder(std::move(ElifLadder)),
    has_else(1), has_finally(1), has_elif(1), ExprAST("",ast_if) {}

    const std::unique_ptr<IfThenExpr>& getIfThen() {return IfThen;}
    const std::unique_ptr<ExprAST>& getElseBlock() {return ElseBlock;}
    const std::vector<std::unique_ptr<IfThenExpr>>& getElifLadder() {return ElifLadder;}
    const std::unique_ptr<ExprAST>& getFinallyBlock() {return FinallyBlock;}
};

}