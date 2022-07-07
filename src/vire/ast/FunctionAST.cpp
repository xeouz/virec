#pragma once

#include <memory>
#include <vector>
#include <string>
#include <map>

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

    std::string const& getName() const {return Callee->value;}
    Viretoken* const getToken() const {return Callee.get();}
    std::unique_ptr<Viretoken> moveToken() {return std::move(Callee);}

    std::vector<std::unique_ptr<ExprAST>> const& getArgs() const {return Args;}
    std::vector<std::unique_ptr<ExprAST>> moveArgs() {return std::move(Args);}
};

class FunctionBaseAST
{
public:
    FunctionBaseAST() {}
    virtual std::string const& getType() const = 0;
    virtual std::string const& getName() const = 0;
    virtual std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const = 0;

    virtual bool is_extern() const {return false;}
    virtual bool is_proto() const {return false;}

    virtual bool isVarDefined(std::string const& name) const {return false;}
    virtual VariableDefAST* const getVar(const std::string& name) const {return nullptr;};

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
    unsigned int getArgCount() const {return Args.size();}
    bool is_proto() const {return true;}
    bool is_type_null() const {return returnType==nullptr;}

    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return Args;}
};

class ExternAST : public FunctionBaseAST
{
    std::unique_ptr<PrototypeAST> Proto;
public:
    int asttype;
    ExternAST(std::unique_ptr<PrototypeAST> Proto) : Proto(std::move(Proto)), asttype(ast_extern) {}

    std::string const& getName() const   {return Proto->getName();}
    std::string const& getType() const   {return Proto->getType();}
    PrototypeAST* const getProto() const {return Proto.get();}
    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return Proto->getArgs();}
    bool is_extern() const {return true;}
};

// FunctionAST - Class for functions which can be called by the user
class FunctionAST : public FunctionBaseAST
{
    std::unique_ptr<PrototypeAST> Proto;
    std::vector<std::unique_ptr<ExprAST>> Statements;
    std::map<std::string, VariableDefAST*> Locals;
public:
    int asttype;
    FunctionAST(std::unique_ptr<PrototypeAST> proto, std::vector<std::unique_ptr<ExprAST>> stmts)
    : Proto(std::move(proto)), Statements(std::move(stmts)), asttype(ast_function) 
    {
        for(auto const& arg : Proto->getArgs())
            Locals[arg->getName()] = arg.get();
    }

    std::string const& getType() const {return Proto->getType();}
    std::string const& getName() const {return Proto->getName();}

    PrototypeAST* const getProto() const {return Proto.get();}
    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return Proto->getArgs();}
    std::vector<std::unique_ptr<ExprAST>> const& getBody() const {return Statements;}
    void insertStatement(std::unique_ptr<ExprAST> Statement) {Statements.insert(Statements.begin(), std::move(Statement));}

    bool isVarDefined(std::string const& name) const {return Locals.count(name)>0;}
    VariableDefAST* const getVar(std::string const& name) const {return Locals.at(name);}
    void addVar(VariableDefAST* const& var) {Locals[var->getName()] = var;}
};

class ReturnExprAST : public ExprAST
{
    std::unique_ptr<ExprAST> Values;
    std::string func_name;
public:
    ReturnExprAST(std::unique_ptr<ExprAST> Values) : Values(std::move(Values)), ExprAST("",ast_return)
    {}

    void setName(std::string name) {func_name = name;}
    std::string const& getName() const {return func_name;}
    ExprAST* const getValue() const {return Values.get();}
};

}