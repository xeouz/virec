#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>

namespace vire
{

class UnionExprAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> members;
    std::unique_ptr<Viretoken> name;

    unsigned int size;
public:
    UnionExprAST(std::vector<std::unique_ptr<ExprAST>> members, std::unique_ptr<Viretoken> name) 
    : members(std::move(members)), name(std::move(name)), ExprAST("void",ast_union)
    {}

    void setSize(unsigned int size)
    {
        this->size=size;
    }

    const std::string& getName() const 
    {
        return name->value;
    }
    const unsigned int getSize() const
    {
        return size;
    }

    const std::vector<std::unique_ptr<ExprAST>>& getMembers() const 
    {
        return members;
    }
};

class StructExprAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> members;
    std::unique_ptr<Viretoken> name;

    unsigned int size;
public:
    StructExprAST(std::vector<std::unique_ptr<ExprAST>> members, std::unique_ptr<Viretoken> name)
    : members(std::move(members)), name(std::move(name)),  ExprAST("void",ast_struct)
    {}

    void setSize(unsigned int size)
    {
        this->size=size;
    }

    const std::string& getName() const 
    {
        return name->value;
    }
    const unsigned int getSize() const 
    {
        return size;
    }

    const std::vector<std::unique_ptr<ExprAST>>& getMembers() const 
    {
        return members;
    }
};

}