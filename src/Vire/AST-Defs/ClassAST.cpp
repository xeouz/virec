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
    std::string Name;
    std::string Parent;
    std::vector<std::unique_ptr<FunctionBaseAST>> Functions;
    std::vector<std::unique_ptr<VariableDefAST>> Variables;
public:
    ClassAST(std::string Name, std::vector<std::unique_ptr<FunctionBaseAST>> Functions)
    : Name(Name), Functions(std::move(Functions)), Parent("") {}
    ClassAST(std::string Name, std::vector<std::unique_ptr<FunctionBaseAST>> Functions
    , std::vector<std::unique_ptr<VariableDefAST>> Variables)
    : Name(Name), Functions(std::move(Functions)), Parent(""), Variables(std::move(Variables))
    {}
    ClassAST(std::string Name, std::vector<std::unique_ptr<FunctionBaseAST>> Functions
    , std::vector<std::unique_ptr<VariableDefAST>> Variables, std::string Parent)
    : Name(Name), Functions(std::move(Functions)), Parent(Parent), Variables(std::move(Variables))
    {}

    template<typename T>
    std::vector<std::unique_ptr<T>> getFunctions()
    {
        std::vector<std::unique_ptr<T>> retCast;

        for(auto& item: Functions)
        {
            auto t=std::unique_ptr<T>(static_cast<T*>(item.get()));
            retCast.push_back(std::move(t));
        }

        return std::move(retCast);
    }
    std::vector<std::unique_ptr<VariableDefAST>> getVariables()
    {
        std::vector<std::unique_ptr<VariableDefAST>> ret;

        for(auto& item:Variables)
        {
            std::unique_ptr<VariableDefAST> var(static_cast<VariableDefAST*>(item.get()));
            ret.push_back(std::move(var));
        }

        return std::move(ret);
    }

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

    const std::string& getParent() const {return Parent;}
    const std::string& getName() const {return Name;}
};

class NewExprAST : public ExprAST
{
    std::string ClassName;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    NewExprAST(const std::string&ClassName, std::vector<std::unique_ptr<ExprAST>> Args)
    : ClassName(ClassName), Args(std::move(Args)), ExprAST("",ast_new) {};

    const std::string& getName() const {return ClassName;}
    std::vector<std::unique_ptr<ExprAST>> getArgs() {return std::move(Args);}
};

class DeleteExprAST : public ExprAST
{
    std::string varName;
public:
    DeleteExprAST(const std::string& varName) : varName(varName), ExprAST("",ast_delete) 
    {}

    const std::string& getName() const {return varName;}
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