#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include "VariableAST.cpp"
#include "FunctionAST.cpp"
#include "Identifier.cpp"

#include <string>
#include <vector>
#include <memory>

namespace vire
{

class ClassAST
{
    std::unique_ptr<Viretoken> Name;
    std::unique_ptr<Viretoken> Parent;
    std::map<std::string, std::unique_ptr<VariableDefAST>> Variables;
    std::map<std::string, std::unique_ptr<FunctionBaseAST>> Functions;
public:
    ClassAST(std::unique_ptr<Viretoken> Name, std::vector<std::unique_ptr<FunctionBaseAST>> funcs
    , std::vector<std::unique_ptr<VariableDefAST>> vars, std::unique_ptr<Viretoken> Parent)
    : Name(std::move(Name)), Parent(std::move(Name))
    {
        unsigned int it=0;
        for(it=0; it<funcs.size(); it++)
        {
            Functions.emplace(funcs[it]->getName(), std::move(funcs[it]));
        }
        for(it=0; it<vars.size(); it++)
        {
            Variables.emplace(vars[it]->getName(), std::move(vars[it]));
        }
    }

    std::vector<FunctionBaseAST const*> getFunctions() const
    {
        std::map<std::string, std::unique_ptr<FunctionBaseAST>>::const_iterator it;
        std::vector<FunctionBaseAST const*> ret;
        for(it=Functions.begin(); it!=Functions.end(); ++it)
        {
            ret.push_back(it->second.get());
        }

        return ret;
    }
    std::vector<VariableDefAST const*> getMembers() const 
    {
        std::map<std::string, std::unique_ptr<VariableDefAST>>::const_iterator it;
        std::vector<VariableDefAST const*> ret;
        for(it=Variables.begin(); it!=Variables.end(); ++it)
        {
            ret.push_back(it->second.get());
        }

        return ret;
    }

    VariableDefAST* const getVariable(std::string varName)
    {
        return Variables[varName].get();
    }

    template<typename T>
    std::unique_ptr<T> getFunction(std::string funcName)
    {
        return (std::unique_ptr<T>)Functions[funcName].get();
    }

    std::string const& getParent() const {return Parent->value;}
    std::string const& getName() const {return Name->value;}
};

class NewExprAST : public ExprAST
{
    std::unique_ptr<Viretoken> ClassName;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    NewExprAST(std::unique_ptr<Viretoken> ClassName, std::vector<std::unique_ptr<ExprAST>> Args)
    : ClassName(std::move(ClassName)), Args(std::move(Args)), ExprAST("",ast_new) {};

    std::string const& getName() const {return ClassName->value;}
    std::vector<std::unique_ptr<ExprAST>> const& getArgs() {return Args;}
};

class DeleteExprAST : public ExprAST
{
    std::unique_ptr<Viretoken> varName;
public:
    DeleteExprAST(std::unique_ptr<Viretoken> varName) : varName(std::move(varName)), ExprAST("",ast_delete) 
    {}

    std::string const& getName() const {return varName->value;}
};

class ClassAccessAST : public ExprAST
{
    std::unique_ptr<ExprAST> parent;
    std::unique_ptr<IdentifierExprAST> child;
public:
    ClassAccessAST(std::unique_ptr<ExprAST> parent, std::unique_ptr<IdentifierExprAST> child)
    : parent(std::move(parent)), child(std::move(child)), ExprAST("",ast_class_access) {}

    ExprAST* const getParent()
    {
        return parent.get();
    }
    IdentifierExprAST* const getChild()
    {
        return child.get();
    }
};

}