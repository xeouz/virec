#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>

namespace vire
{

class IfThenExpr : public ExprAST
{
    std::unique_ptr<ExprAST> condition;
    std::vector<std::unique_ptr<ExprAST>> ThenBlock;
public:
    IfThenExpr(std::unique_ptr<ExprAST> Condition, std::vector<std::unique_ptr<ExprAST>> ThenBlock)
    : condition(std::move(Condition)), ThenBlock(std::move(ThenBlock)), ExprAST("",ast_if)
    {}

    ExprAST* const getCondition() 
    {
        return condition.get();
    }
    std::unique_ptr<ExprAST> moveCondition()
    {
        return std::move(condition);
    }
    void setCondition(std::unique_ptr<ExprAST> Condition)
    {
        condition=std::move(Condition);
    }
    
    std::vector<std::unique_ptr<ExprAST>> const& getThenBlock() 
    {
        return ThenBlock;
    }
};

class IfExprAST : public ExprAST
{
    std::unique_ptr<IfThenExpr> IfThen;
    std::vector<std::unique_ptr<IfThenExpr>> ElifLadder;
public:

    IfExprAST(std::unique_ptr<IfThenExpr> IfThen, std::vector<std::unique_ptr<IfThenExpr>> ElifLadder)
    : IfThen(std::move(IfThen)), ElifLadder(std::move(ElifLadder)), ExprAST("",ast_ifelse)
    {}

    ExprAST* const getCondition() {return IfThen->getCondition();}
    std::vector<std::unique_ptr<ExprAST>> const& getThenBlock() {return IfThen->getThenBlock();}
    IfThenExpr* const getIfThen() {return IfThen.get();}
    std::vector<std::unique_ptr<IfThenExpr>> const& getElifLadder() {return ElifLadder;}
};

}