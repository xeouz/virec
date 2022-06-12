#pragma once

#include <memory>

#include "ASTType.hpp"
#include "ExprAST.cpp"
#include "../Lex/token.cpp"

namespace vire
{
// UnaryExprAST - Class for a unary operator, eg - `!`
class UnaryExprAST : public ExprAST
{
    std::unique_ptr<Viretoken> Op;
    std::unique_ptr<ExprAST> Expr;
public:
    UnaryExprAST(std::unique_ptr<Viretoken> Op, std::unique_ptr<ExprAST> Expr)
    : Op(std::move(Op)), Expr(std::move(Expr)), ExprAST("void",ast_unop) {}

    const std::unique_ptr<Viretoken>& getOp() const { return Op; }
    const std::unique_ptr<ExprAST>& getExpr() const { return Expr; }
};

// BinaryExprAST - Class for a binary operator, eg - `+`
class BinaryExprAST : public ExprAST
{
    std::unique_ptr<Viretoken> Op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:

    BinaryExprAST(std::unique_ptr<Viretoken> Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
    : Op(std::move(Op)), LHS(std::move(LHS)), RHS(std::move(RHS)), ExprAST("void",ast_binop) {}

    const std::unique_ptr<Viretoken>& getOp() const {return Op;}
    const std::unique_ptr<ExprAST>& getLHS() const {return LHS;}
    const std::unique_ptr<ExprAST>& getRHS() const {return RHS;}
    std::unique_ptr<ExprAST> moveLHS() {return std::move(LHS);}
    std::unique_ptr<ExprAST> moveRHS() {return std::move(RHS);}

};

}