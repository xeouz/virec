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
    proto::IName name;
    proto::IName parent;
    std::unique_ptr<VToken> name_token;
    std::unique_ptr<VToken> parent_token;
    std::map<std::string, std::unique_ptr<VariableDefAST>> Variables;
    std::map<std::string, std::unique_ptr<FunctionBaseAST>> Functions;
public:
    ClassAST(std::unique_ptr<VToken> name_token, std::vector<std::unique_ptr<FunctionBaseAST>> funcs
    , std::vector<std::unique_ptr<VariableDefAST>> vars, std::unique_ptr<VToken> parent_token)
    : name(name_token->value), parent(parent_token->value), name_token(std::move(name_token)), parent_token(std::move(parent_token))
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

    std::string const& getParent() const {return parent.get();}
    std::string const& getName() const {return name.get();}
};

class NewExprAST : public ExprAST
{
    proto::IName class_name;
    std::unique_ptr<VToken> class_name_token;
    std::vector<std::unique_ptr<ExprAST>> args;
public:
    NewExprAST(std::unique_ptr<VToken> class_name_token, std::vector<std::unique_ptr<ExprAST>> args)
    : class_name(class_name_token->value), class_name_token(std::move(class_name_token)), args(std::move(args)), ExprAST("",ast_new) {};

    std::string const& getName() const {return class_name.get();}
    std::vector<std::unique_ptr<ExprAST>> const& getArgs() {return args;}
};

class DeleteExprAST : public ExprAST
{
    proto::IName var_name;
    std::unique_ptr<VToken> var_name_token;
public:
    DeleteExprAST(std::unique_ptr<VToken> var_name_token) 
    : var_name(var_name_token->value), var_name_token(std::move(var_name_token)), ExprAST("",ast_delete) 
    {}

    std::string const& getName() const {return var_name.get();}
};

}