#pragma once

#include <memory>

#include "ASTType.hpp"
#include "ExprAST.cpp"

namespace vire
{

// UnaryExprAST - Class for a unary operator, eg - `!`
class UnaryExprAST : public ExprAST
{
    std::unique_ptr<VToken> op;
    std::unique_ptr<ExprAST> Expr;
public:
    UnaryExprAST(std::unique_ptr<VToken> op, std::unique_ptr<ExprAST> Expr)
    : op(std::move(op)), Expr(std::move(Expr)), ExprAST("void",ast_unop) {}

    VToken* const getop() const { return op.get();   }
    ExprAST* const getExpr() const { return Expr.get(); }
};

// BinaryExprAST - Class for a binary operator, eg - `+`
class BinaryExprAST : public ExprAST
{
    std::unique_ptr<VToken> op;
    std::unique_ptr<ExprAST> lhs, rhs;
public:

    BinaryExprAST(std::unique_ptr<VToken> op, std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs)
    : op(std::move(op)), lhs(std::move(lhs)), rhs(std::move(rhs)), ExprAST("bool",ast_binop) {}

    VToken* const getOp() const {return op.get();}
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

// IncrementDecrementAST - Class for a increment or decrement operation
class IncrementDecrementAST : public ExprAST
{
    std::unique_ptr<ExprAST> expr;
    bool is_pre;
    bool is_increment;
public:
    IncrementDecrementAST(std::unique_ptr<ExprAST> expr, bool is_pre, bool is_increment)
    : expr(std::move(expr)), is_pre(is_pre), is_increment(is_increment), ExprAST("void", ast_incrdecr)
    {
    }

    ExprAST* const getExpr() const
    {
        return expr.get();
    }

    bool isPre() const
    {
        return is_pre;
    }
    bool isIncrement() const
    {
        return is_increment;
    }
};

}