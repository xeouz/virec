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
};

// BinaryExprAST - Class for a binary operator, eg - `+`
class BinaryExprAST : public ExprAST
{
    std::unique_ptr<Viretoken> Op;
    std::unique_ptr<ExprAST> LHS, RHS;
public:

    BinaryExprAST(std::unique_ptr<Viretoken> Op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
    : Op(std::move(Op)), LHS(std::move(LHS)), RHS(std::move(RHS)), ExprAST("void",ast_binop) {}

    template<typename T>
    T* getLHS() const {return dynamic_cast<T*>(LHS.get());}
    template<typename T>
    T* getRHS() const {return dynamic_cast<T*>(RHS.get());}

    template<typename T>
    std::unique_ptr<T> moveLHS() {
        std::unique_ptr<T> newLHS(static_cast<T*>(LHS.get()));
        return std::move(newLHS);
    }

    template<typename T>
    std::unique_ptr<T> moveRHS() {
        std::unique_ptr<T> newRHS(static_cast<T*>(RHS.get()));
        return std::move(newRHS);
    }
};
}