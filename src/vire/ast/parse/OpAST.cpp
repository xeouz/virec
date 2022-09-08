#pragma once

#include <memory>

#include "ASTType.hpp"
#include "ExprAST.cpp"

namespace vire
{

// UnaryExprAST - Class for a unary operator, eg - `!`
class UnaryExprAST : public ExprAST
{
    std::unique_ptr<Viretoken> op;
    std::unique_ptr<ExprAST> Expr;
public:
    UnaryExprAST(std::unique_ptr<Viretoken> op, std::unique_ptr<ExprAST> Expr)
    : op(std::move(op)), Expr(std::move(Expr)), ExprAST("void",ast_unop) {}

    Viretoken* const getop() const { return op.get();   }
    ExprAST* const getExpr() const { return Expr.get(); }
};

// BinaryExprAST - Class for a binary operator, eg - `+`
class BinaryExprAST : public ExprAST
{
    std::unique_ptr<Viretoken> op;
    std::unique_ptr<ExprAST> lhs, rhs;
public:

    BinaryExprAST(std::unique_ptr<Viretoken> op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
    : op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)), ExprAST("bool",ast_binop) {}

    Viretoken* const getOp() const {return op.get();}
    ExprAST* const getLHS() const {return lhs.get();}
    ExprAST* const getRHS() const {return rhs.get();}
    std::unique_ptr<ExprAST> moveLHS() {return std::move(lhs);}
    std::unique_ptr<ExprAST> moveRHS() {return std::move(rhs);}

    types::Base* getOpType()
    {
        switch (op->type)
        {
            case tok_plus:  return nullptr;
            case tok_minus: return nullptr;
            case tok_mul:   return nullptr;
            case tok_div:   return nullptr;
            case tok_mod:   return nullptr;
        default:
            return type.get();
        }
    }
};

}