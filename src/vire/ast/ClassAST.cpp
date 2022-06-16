#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include "VariableAST.cpp"
#include "FunctionAST.cpp"

#include <string>
#include <vector>
#include <memory>

namespace vire
{

class ClassAST
{
    std::unique_ptr<Viretoken> Name;
    std::unique_ptr<Viretoken> Parent;
    std::vector<std::unique_ptr<FunctionBaseAST>> Functions;
    std::vector<std::unique_ptr<VariableDefAST>> Variables;
public:
    ClassAST(std::unique_ptr<Viretoken> Name, std::vector<std::unique_ptr<FunctionBaseAST>> Functions)
    : Name(std::move(Name)), Functions(std::move(Functions)), Parent(nullptr) {}
    ClassAST(std::unique_ptr<Viretoken> Name, std::vector<std::unique_ptr<FunctionBaseAST>> Functions
    , std::vector<std::unique_ptr<VariableDefAST>> Variables)
    : Name(std::move(Name)), Functions(std::move(Functions)), Parent(nullptr), Variables(std::move(Variables))
    {}
    ClassAST(std::unique_ptr<Viretoken> Name, std::vector<std::unique_ptr<FunctionBaseAST>> Functions
    , std::vector<std::unique_ptr<VariableDefAST>> Variables, std::unique_ptr<Viretoken> Parent)
    : Name(std::move(Name)), Functions(std::move(Functions)), Parent(std::move(Name)), Variables(std::move(Variables))
    {}

    const std::vector<std::unique_ptr<FunctionBaseAST>>& getFunctions() const {return Functions;}
    const std::vector<std::unique_ptr<VariableDefAST>>& getMembers() const {return Variables;}

    std::unique_ptr<VariableDefAST> getVariable(std::string varName)
    {
        for(auto& item:Variables)
        {
            if(item->getName() == varName)
            {
                std::unique_ptr<VariableDefAST> var(static_cast<VariableDefAST*>(item.get()));
                return std::move(var);
            }
        }

        return nullptr;
    }

    template<typename T>
    std::unique_ptr<T> getFunction(std::string funcName)
    {
        for(int i=0; i<Functions.size(); ++i)
        {
            if(Functions[i]->getName() == funcName)
            {
                T* func=dynamic_cast<T*>(Functions[i].get());
                auto uptr=std::make_unique<T>(func);
                delete func;
                return std::move(uptr);
            }
        }
    }

    const std::string& getParent() const {return Parent->value;}
    const std::string& getName() const {return Name->value;}
};

class NewExprAST : public ExprAST
{
    std::unique_ptr<Viretoken> ClassName;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    NewExprAST(std::unique_ptr<Viretoken> ClassName, std::vector<std::unique_ptr<ExprAST>> Args)
    : ClassName(std::move(ClassName)), Args(std::move(Args)), ExprAST("",ast_new) {};

    const std::string& getName() const {return ClassName->value;}
    std::vector<std::unique_ptr<ExprAST>> getArgs() {return std::move(Args);}
};

class DeleteExprAST : public ExprAST
{
    std::unique_ptr<Viretoken> varName;
public:
    DeleteExprAST(std::unique_ptr<Viretoken> varName) : varName(std::move(varName)), ExprAST("",ast_delete) 
    {}

    const std::string& getName() const {return varName->value;}
};

class ClassAccessAST : public ExprAST
{
    std::unique_ptr<ExprAST> parent;
    std::unique_ptr<ExprAST> child;
public:
    ClassAccessAST(std::unique_ptr<ExprAST> parent, std::unique_ptr<ExprAST> child)
    : parent(std::move(parent)), child(std::move(child)), ExprAST("",ast_class_access) {}

    std::unique_ptr<ExprAST> getParent() {return std::move(parent);}
    std::unique_ptr<ExprAST> getChild() {return std::move(child);}
};

}