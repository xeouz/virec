#pragma once

#include <memory>
#include <vector>
#include <string>

#include "ASTType.hpp"
#include "ExprAST.cpp"
#include "VariableAST.cpp"

namespace vire
{
// CallExprAST - Class for function calls, eg - `print()`
class CallExprAST : public ExprAST
{
    std::string Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(const std::string& Callee, std::vector<std::unique_ptr<ExprAST>> Args)
    : Callee(Callee), Args(std::move(Args)), ExprAST("void",ast_call){}

    const std::string& getName() const {return Callee;}
    std::vector<std::unique_ptr<ExprAST>> getArgs() {return std::move(Args);}
};

class FunctionBaseAST
{
public:
    virtual std::string getName() const {return std::string();}
};

// PrototypeAST - Class for prototype functions, captures function name and args
class PrototypeAST : public FunctionBaseAST
{
    std::string Name;
    std::string returnType;
    std::vector<std::unique_ptr<VariableDefAST>> Args;
public:
    int asttype;
    PrototypeAST(const std::string& Name, std::vector<std::unique_ptr<VariableDefAST>> Args, std::string returnType)
    : Name(Name), Args(std::move(Args)), returnType(returnType) {}
    PrototypeAST(const std::string& Name, std::vector<std::unique_ptr<VariableDefAST>> Args)
    : Name(Name), Args(std::move(Args)), returnType("void") {}
    
    const std::string& getType() const {return returnType;}
    std::string getName() const {return Name;}

    const std::vector<std::unique_ptr<VariableDefAST>>& getArgs() const {return Args;}
};

class ExternAST : public FunctionBaseAST
{
    std::unique_ptr<PrototypeAST> Proto;
public:
    int asttype;
    ExternAST(std::unique_ptr<PrototypeAST> Proto) : Proto(std::move(Proto)), asttype(ast_extern) {}

    std::string getName() const {return Proto->getName();}
};

// FunctionAST - Class for functions which can be called by the user
class FunctionAST : public FunctionBaseAST
{
    std::unique_ptr<PrototypeAST> Proto;
    std::vector<std::unique_ptr<ExprAST>> Statements;
public:
    int asttype;
    FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::vector<std::unique_ptr<ExprAST>> Statements)
    : Proto(std::move(Proto)), Statements(std::move(Statements)), asttype(ast_function) {}

    const std::string& getType() const {return Proto->getType();}
    std::string getName() const {return Proto->getName();}
};


class ReturnExprAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> Values;
public:
    ReturnExprAST(std::vector<std::unique_ptr<ExprAST>> Values) : Values(std::move(Values)), ExprAST("",ast_return)
    {}

    const std::vector<std::unique_ptr<ExprAST>>& getValues() {return Values;}
};

}