#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>

namespace vire
{

class UnionExprAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> Members;
    std::string Name;
    char is_anonymous;
public:
    UnionExprAST(std::vector<std::unique_ptr<ExprAST>> Members) : Members(std::move(Members)), Name(""), is_anonymous(1)
    , ExprAST("void",ast_union) {}
    UnionExprAST(std::vector<std::unique_ptr<ExprAST>> Members, std::string Name) 
    : Members(std::move(Members)), Name(Name), is_anonymous(0), ExprAST("void",ast_union)
    {}

    const std::string& getName() const {return Name;}

    const std::vector<std::unique_ptr<ExprAST>>& getMembers() const {return Members;}
};

class StructExprAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> Members;
    std::string Name;
    char is_anonymous;
public:
    StructExprAST(std::vector<std::unique_ptr<ExprAST>> Members)
    : Members(std::move(Members)), Name(""), is_anonymous(1), ExprAST("void",ast_struct) {}
    StructExprAST(std::vector<std::unique_ptr<ExprAST>> Members, std::string Name)
    : Members(std::move(Members)), Name(Name), is_anonymous(0), ExprAST("void",ast_struct) {}

    const std::string& getName() const {return Name;}

    const std::vector<std::unique_ptr<ExprAST>>& getMembers() const {return Members;}
};

}