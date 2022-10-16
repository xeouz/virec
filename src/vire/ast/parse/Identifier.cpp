#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

namespace vire
{

class IdentifierExprAST : public ExprAST
{
    std::string name;
public:
    bool is_const;
    IdentifierExprAST(std::unique_ptr<Viretoken> name, bool is_const=false, int asttype=ast_var) 
    : name(name->value), ExprAST("", asttype), is_const(is_const)
    {
        setToken(std::move(name));
    }

    virtual std::string const getName() const
    {
        return name;
    }
    virtual std::unique_ptr<Viretoken> moveToken()
    {
        return std::move(token);
    }
    virtual void setName(std::string const& _name)
    {
        name=_name;
    }
};

// VariableExprAST - Class for referencing a variable, eg - `myvar`
class VariableExprAST : public IdentifierExprAST
{
    std::string name;
public:
    VariableExprAST(std::unique_ptr<Viretoken> name) : 
    IdentifierExprAST(std::move(name))
    {
        this->name=this->getToken()->value;
    } 
};

class TypeAccessAST : public IdentifierExprAST
{
    std::unique_ptr<ExprAST> parent;
    std::unique_ptr<IdentifierExprAST> child;
public:
    TypeAccessAST(std::unique_ptr<ExprAST> _parent, std::unique_ptr<IdentifierExprAST> _child)
    : parent(std::move(_parent)), child(std::move(_child)), 
    IdentifierExprAST(Viretoken::construct(""), false, ast_type_access)
    {
        if(child->asttype==ast_type_access)
        {
            auto* cast_child=(TypeAccessAST*)child.get();
            auto* cast_child_child=(VariableExprAST*)cast_child->getParent();
            setName(cast_child_child->getName());
            setToken(Viretoken::construct(cast_child_child->getName()));
        }
        else
        {
            setName(child->getName());
            setToken(Viretoken::construct(child->getName()));
        }
    }

    ExprAST* const getParent() const
    {
        return parent.get();
    }
    IdentifierExprAST* const getChild() const
    {
        return child.get();
    }

};

}