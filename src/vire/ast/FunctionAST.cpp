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
    std::unique_ptr<Viretoken> Callee;
    std::vector<std::unique_ptr<ExprAST>> Args;
public:
    CallExprAST(std::unique_ptr<Viretoken> Callee, std::vector<std::unique_ptr<ExprAST>> Args)
    : Callee(std::move(Callee)), Args(std::move(Args)), ExprAST("void",ast_call){}

    const std::string& getName() const {return Callee->value;}
    const std::unique_ptr<Viretoken>& getToken() const {return Callee;}
    std::unique_ptr<Viretoken> moveToken() {return std::move(Callee);}

    std::vector<std::unique_ptr<ExprAST>> const& getArgs() const {return Args;}
    std::vector<std::unique_ptr<ExprAST>> moveArgs() {return std::move(Args);}
};

class FunctionBaseAST
{
public:
    virtual std::string const& getType() const = 0;
    virtual std::string const& getName() const = 0;
    virtual std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const = 0;

    virtual ~FunctionBaseAST() {}
};

// PrototypeAST - Class for prototype functions, captures function name and args
class PrototypeAST : public FunctionBaseAST
{
    std::unique_ptr<Viretoken> Name;
    std::unique_ptr<Viretoken> returnType;
    std::vector<std::unique_ptr<VariableDefAST>> Args;
public:
    int asttype;
    PrototypeAST(std::unique_ptr<Viretoken> Name, std::vector<std::unique_ptr<VariableDefAST>> Args, std::unique_ptr<Viretoken> returnType)
    : Name(std::move(Name)), Args(std::move(Args)), returnType(std::move(returnType)) {}
    PrototypeAST(std::unique_ptr<Viretoken> Name, std::vector<std::unique_ptr<VariableDefAST>> Args)
    : Name(std::move(Name)), Args(std::move(Args)), returnType(nullptr) {}
    
    std::string const& getType() const {return returnType->value;}
    std::string const& getName() const {return Name->value;}

    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return Args;}
};

class ExternAST : public FunctionBaseAST
{
    std::unique_ptr<PrototypeAST> Proto;
public:
    int asttype;
    ExternAST(std::unique_ptr<PrototypeAST> Proto) : Proto(std::move(Proto)), asttype(ast_extern) {}

    std::string const& getName() const {return Proto->getName();}
    std::string const& getType() const {return Proto->getType();}
    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return Proto->getArgs();}
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

    std::string const& getType() const {return Proto->getType();}
    std::string const& getName() const {return Proto->getName();}

    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return Proto->getArgs();}
};

class ReturnExprAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> Values;
    std::string func_name;
public:
    ReturnExprAST(std::vector<std::unique_ptr<ExprAST>> Values) : Values(std::move(Values)), ExprAST("",ast_return)
    {}

    const std::string& getName() const {return func_name;}
    std::vector<std::unique_ptr<ExprAST>> const& getValues() const {return Values;}
};

}