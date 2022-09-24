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
    std::unique_ptr<Viretoken> callee;
    std::vector<std::unique_ptr<ExprAST>> args;
public:
    CallExprAST(std::unique_ptr<Viretoken> callee, std::vector<std::unique_ptr<ExprAST>> args)
    : callee(std::move(callee)), args(std::move(args)), ExprAST("void",ast_call){}

    std::string const& getName() const
    {
        return callee->value;
    }
    Viretoken* const getToken() const
    {
        return callee.get();
    }
    std::unique_ptr<Viretoken> moveToken()
    {
        return std::move(callee);
    }

    std::vector<std::unique_ptr<ExprAST>> const& getArgs() const 
    {
        return args;
    }
    std::vector<std::unique_ptr<ExprAST>> moveArgs() 
    {
        return std::move(args);
    }
    void setArgs(std::vector<std::unique_ptr<ExprAST>> _args)
    {
        args=std::move(_args);
    }
};

class FunctionBaseAST
{
protected:
    std::unique_ptr<types::Base> return_type;
public:
    FunctionBaseAST(std::string return_name)
    :   return_type(types::construct(return_name))
    {}
    
    types::Base* getReturnType() const { return return_type.get(); }
    std::unique_ptr<types::Base> moveReturnType() { return std::move(return_type); }
    void setReturnType(std::unique_ptr<types::Base> t) { this->return_type=std::move(t); }

    virtual std::string      const getName()       const = 0;
    virtual std::string      const getReturnName() const = 0;

    virtual Viretoken* const getNameToken()        const = 0;
    virtual Viretoken* const getReturnNameToken()  const = 0;

    virtual std::unique_ptr<Viretoken> moveNameToken()       = 0;
    virtual std::unique_ptr<Viretoken> moveReturnNameToken() = 0;

    virtual std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const = 0;

    virtual bool is_extern() const { return false; }
    virtual bool is_proto()  const { return false; }

    //virtual bool isVarDefined(std::string const& name) const = 0;
    //virtual VariableDefAST* const getVar(const std::string& name) const = 0;

    virtual ~FunctionBaseAST() = default;
};

// PrototypeAST - Class for prototype functions, captures function name and args
class PrototypeAST : public FunctionBaseAST
{
    std::unique_ptr<Viretoken> name; 
    std::unique_ptr<Viretoken> return_name;
    std::vector<std::unique_ptr<VariableDefAST>> args;
public:
    int asttype;

    PrototypeAST(std::unique_ptr<Viretoken> name, 
    std::vector<std::unique_ptr<VariableDefAST>> args, 
    std::unique_ptr<Viretoken> return_type)
    : FunctionBaseAST(return_type->value), args(std::move(args)), asttype(ast_proto),
    name(std::move(name)), return_name(std::move(return_type))
    {}

    std::string const getName()       const { return getNameToken()->value; }
    std::string const getReturnName() const { return getReturnNameToken()->value; }

    Viretoken* const getNameToken()        const { return name.get(); }
    Viretoken* const getReturnNameToken()  const { return return_name.get(); }

    std::unique_ptr<Viretoken> moveNameToken()       { return std::move(name); }
    std::unique_ptr<Viretoken> moveReturnNameToken() { return std::move(return_name); }
    
    unsigned int getArgCount() const {return args.size();}
    
    bool is_proto() const {return true;}
    bool is_type_null() const {return return_type==nullptr;}

    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return args;}
};

class ExternAST : public FunctionBaseAST
{
    std::unique_ptr<PrototypeAST> proto;
public:
    int asttype;

    ExternAST(std::unique_ptr<PrototypeAST> prototype)
    : FunctionBaseAST(prototype->getReturnName()), 
    asttype(ast_extern), proto(std::move(prototype))
    {}

    std::string const getName()       const { return proto->getName(); }
    std::string const getReturnName() const { return proto->getReturnName(); }

    Viretoken* const getNameToken()        const { return proto->getNameToken(); }
    Viretoken* const getReturnNameToken()  const { return proto->getReturnNameToken(); }

    std::unique_ptr<Viretoken> moveNameToken()       { return proto->moveNameToken(); }
    std::unique_ptr<Viretoken> moveReturnNameToken() { return proto->moveReturnNameToken(); }
    
    bool is_extern() const {return true;}
    bool is_type_null() const {return return_type==nullptr;}

    PrototypeAST* const getProto() const {return proto.get();}
    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return proto->getArgs();}
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
    : FunctionBaseAST(prototype->getReturnName()), 
    asttype(ast_function), proto(std::move(prototype)), statements(std::move(stmts))
    {
        for(auto const& arg : proto->getArgs())
            locals[arg->getName()] = arg.get();
    }

    // Getter Functions
    PrototypeAST*                                const getProto() const { return proto.get(); }
    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const { return proto->getArgs(); }
    std::vector<std::unique_ptr<ExprAST>>        const& getBody() const { return statements; }

    std::string const getName()       const { return proto->getName(); }
    std::string const getReturnName() const { return proto->getReturnName(); }

    Viretoken* const getNameToken()        const { return proto->getNameToken(); }
    Viretoken* const getReturnNameToken()  const { return proto->getReturnNameToken(); }

    std::unique_ptr<Viretoken> moveNameToken()       { return proto->moveNameToken(); }
    std::unique_ptr<Viretoken> moveReturnNameToken() { return proto->moveReturnNameToken(); }

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
    std::unique_ptr<ExprAST> expr;
    std::string func_name;
public:
    ReturnExprAST(std::unique_ptr<ExprAST> expr) : expr(std::move(expr)), ExprAST("",ast_return)
    {}

    void setName(std::string name) {func_name = name;}
    std::string const& getName() const {return func_name;}
    ExprAST* const getValue() const {return expr.get();}
    std::unique_ptr<ExprAST> const moveValue() { return std::move(expr); }
    void setValue(std::unique_ptr<ExprAST> t) { expr=std::move(t); }
};

}