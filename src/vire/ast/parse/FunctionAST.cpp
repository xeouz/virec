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
protected:
    std::unique_ptr<Viretoken> name;

    std::unique_ptr<Viretoken> return_name;
    std::unique_ptr<types::Base> return_type;
public:
    FunctionBaseAST(Viretoken* name, Viretoken* return_name)
    :   name(std::make_unique<Viretoken>(*name))
    {
        this->return_name=std::make_unique<Viretoken>(*return_name);
        
        std::unique_ptr<types::Base> t(types::construct(this->return_name->value));
        return_type=std::move(t);
    }

    std::string      const& getName() const       { return name->value; }
    types::TypeNames const& getReturnType() const { return return_type->getType(); }
    std::string      const& getReturnName() const { return return_name->value; }

    Viretoken* const getNameToken() const {return name.get();}
    Viretoken* const getReturnNameToken() const {return return_name.get();}

    virtual std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const = 0;

    virtual bool is_extern() const { return false; }
    virtual bool is_proto() const { return false; }

    virtual bool isVarDefined(std::string const& name) const = 0;
    virtual VariableDefAST* const getVar(const std::string& name) const = 0;

    virtual ~FunctionBaseAST() {}
};

// PrototypeAST - Class for prototype functions, captures function name and args
class PrototypeAST : public FunctionBaseAST
{
    std::vector<std::unique_ptr<VariableDefAST>> args;
public:
    int asttype;

    PrototypeAST(Viretoken* name, std::vector<std::unique_ptr<VariableDefAST>> args, Viretoken* return_type)
    : FunctionBaseAST(name, return_type), args(std::move(args)), asttype(ast_proto) {}
    
    unsigned int getArgCount() const {return args.size();}
    
    bool is_proto() const {return true;}
    bool is_type_null() const {return return_type==nullptr;}

    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return args;}
};

class ExternAST : public FunctionBaseAST
{
    std::vector<std::unique_ptr<VariableDefAST>> args;
public:
    int asttype;

    ExternAST(Viretoken* name, std::vector<std::unique_ptr<VariableDefAST>> args, Viretoken* return_type)
    : FunctionBaseAST(name, return_type), args(std::move(args)), asttype(ast_extern) {}
    
    unsigned int getArgCount() const {return args.size();}
    
    bool is_extern() const {return true;}
    bool is_type_null() const {return return_type==nullptr;}

    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return args;}
};

// FunctionAST - Class for functions which can be called by the user
class FunctionAST : public FunctionBaseAST
{
    std::unique_ptr<PrototypeAST> proto;
    std::vector<std::unique_ptr<ExprAST>> statements;
    std::map<std::string, VariableDefAST*> locals;
public:
    int asttype;
    
    FunctionAST(std::unique_ptr<PrototypeAST> prototype, std::vector<std::unique_ptr<ExprAST>> stmts)
    : proto(std::move(prototype)), statements(std::move(stmts)),
    FunctionBaseAST(proto->getNameToken(),proto->getReturnNameToken()), asttype(ast_function) 
    {
        for(auto const& arg : proto->getArgs())
            locals[arg->getName()] = arg.get();
    }

    // Getter Functions
    PrototypeAST*                                const getProto() const { return proto.get(); }
    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const { return proto->getArgs(); }
    std::vector<std::unique_ptr<ExprAST>>        const& getBody() const { return statements; }

    // Block-based Functions
    void insertStatement(std::unique_ptr<ExprAST> statement) 
    { statements.insert( statements.begin(), std::move(statement) ); }

    // Variable-based Functions
    bool isVarDefined(std::string const& name)            const { return locals.count(name)>0; }
    VariableDefAST* const getVar(std::string const& name) const { return locals.at(name); }
    void addVar(VariableDefAST* const& var) {locals[var->getName()] = var;}
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