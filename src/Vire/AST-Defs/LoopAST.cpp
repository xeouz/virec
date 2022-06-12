#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>

namespace vire
{

class ForExprAST : public ExprAST
{
    std::unique_ptr<ExprAST> initExpr;
    std::unique_ptr<ExprAST> condExpr;
    std::unique_ptr<ExprAST> incrExpr;

    std::vector<std::unique_ptr<ExprAST>> body;
public:
    ForExprAST(std::unique_ptr<ExprAST> init, std::unique_ptr<ExprAST> cond, std::unique_ptr<ExprAST> incr,
    std::vector<std::unique_ptr<ExprAST>> body) :
    initExpr(std::move(init)), condExpr(std::move(cond)), incrExpr(std::move(incr)), body(std::move(body))
    , ExprAST("void",ast_for) 
    {}

    const std::unique_ptr<ExprAST>& getInit() const { return initExpr; }
    const std::unique_ptr<ExprAST>& getCond() const { return condExpr; }
    const std::unique_ptr<ExprAST>& getIncr() const { return incrExpr; }

    std::unique_ptr<ExprAST> moveInit() {return std::move(initExpr);}
    std::unique_ptr<ExprAST> moveCond() {return std::move(condExpr);}
    std::unique_ptr<ExprAST> moveIncr() {return std::move(incrExpr);}

    std::vector<std::unique_ptr<ExprAST>> getBody() {return std::move(body);}
};

class WhileExprAST : public ExprAST
{
    std::unique_ptr<ExprAST> condExpr;
    std::vector<std::unique_ptr<ExprAST>> body;
public:
    WhileExprAST(std::unique_ptr<ExprAST> cond, std::vector<std::unique_ptr<ExprAST>> Stms) 
    : condExpr(std::move(cond)), body(std::move(Stms)), ExprAST("void",ast_while) 
    {}

    std::unique_ptr<ExprAST> getCond() {return std::move(condExpr);}
    
    std::vector<std::unique_ptr<ExprAST>> getBody() {return std::move(body);}
};

class BreakExprAST : public ExprAST
{
    std::unique_ptr<ExprAST> after_break;
public:
    char is_after;
    BreakExprAST() : ExprAST("",ast_break), is_after(0) {}
    BreakExprAST(std::unique_ptr<ExprAST> after_break) 
    : after_break(std::move(after_break)), ExprAST("",ast_break), is_after(1)
    {}
};

class ContinueExprAST : public ExprAST
{
    std::unique_ptr<ExprAST> after_cont;
public:
    char is_after;
    ContinueExprAST() : ExprAST("",ast_continue), is_after(0) {}
    ContinueExprAST(std::unique_ptr<ExprAST> after_cont)
    : after_cont(std::move(after_cont)), ExprAST("",ast_continue), is_after(1)
    {}
};

}