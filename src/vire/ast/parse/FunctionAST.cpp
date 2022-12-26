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

class ReturnExprAST;

// CallExprAST - Class for function calls, eg - `print()`
class CallExprAST : public ExprAST
{
    proto::IName callee;
    std::unique_ptr<VToken> callee_token;
    std::vector<std::unique_ptr<ExprAST>> args;
public:
    CallExprAST(std::unique_ptr<VToken> callee_token, std::vector<std::unique_ptr<ExprAST>> args)
    : callee(callee_token->value), callee_token(std::move(callee_token)), args(std::move(args)), ExprAST("void",ast_call)
    {}

    proto::IName const& getIName() const
    {
        return callee;
    }
    std::string const& getName() const
    {
        return callee.get();
    }
    VToken* const getToken() const
    {
        return callee_token.get();
    }
    std::unique_ptr<VToken> moveToken()
    {
        return std::move(callee_token);
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

// FunctionBaseAST - Base Class for the functions
class FunctionBaseAST
{
protected:
    std::unique_ptr<types::Base> return_type;
public:
    FunctionBaseAST(std::string return_name)
    :   return_type(types::construct(return_name))
    {}
    FunctionBaseAST(types::Base* type)
    :   return_type(types::copyType(type))
    {}
    FunctionBaseAST(std::unique_ptr<types::Base> type)
    : return_type(std::move(type))
    {}
    
    types::Base* getReturnType() const { return return_type.get(); }
    void setReturnType(std::unique_ptr<types::Base> t) { this->return_type=std::move(t); }
    void setReturnType(types::Base* t) { this->return_type=types::copyType(t); }

    virtual proto::IName const& getIName() const = 0;

    virtual std::string const getName() const = 0;
    virtual VToken* const getNameToken() const = 0;

    virtual void setName(std::string const& name) = 0;
    virtual void setName(proto::IName const& name) = 0;

    virtual std::unique_ptr<VToken> moveNameToken() = 0;

    virtual std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const = 0;

    virtual bool is_extern() const { return false; }
    virtual bool is_proto()  const { return false; }

    virtual void doesRequireSelfRef(bool val) = 0;
    virtual bool doesRequireSelfRef() const = 0;

    virtual void isConstructor(bool val) {}
    virtual bool isConstructor() const { return false; }
    //virtual bool isVariableDefined(std::string const& name) const = 0;
    //virtual VariableDefAST* const getVariable(const std::string& name) const = 0;

    virtual ~FunctionBaseAST() = default;
};

// PrototypeAST - Class for prototype functions, captures function name and args
class PrototypeAST : public FunctionBaseAST
{
    proto::IName name;
    std::unique_ptr<VToken> name_token;
    std::vector<std::unique_ptr<VariableDefAST>> args;
    bool is_constructor;
    bool requires_selfref;
public:
    int asttype;

    PrototypeAST(std::unique_ptr<VToken> name, std::vector<std::unique_ptr<VariableDefAST>> args, std::unique_ptr<types::Base> return_type, bool requires_selfref=false, bool is_constructor=false)
    : FunctionBaseAST(std::move(return_type)), args(std::move(args)), asttype(ast_proto), requires_selfref(requires_selfref), is_constructor(is_constructor),
    name(name->value), name_token(std::move(name))
    {}

    proto::IName const& getIName()    const { return name; }
    std::string const getName()       const { return name.get(); }
    VToken* const getNameToken()      const { return name_token.get(); }

    void setName(std::string const& nname) { name=proto::IName(nname); }
    void setName(proto::IName const& nname) { name=nname; }

    std::unique_ptr<VToken> moveNameToken() { return std::move(name_token); }
    
    unsigned int getArgCount() const {return args.size();}
    
    bool is_proto() const {return true;}
    bool is_type_null() const {return FunctionBaseAST::return_type==nullptr;}

    void doesRequireSelfRef(bool val) { requires_selfref=val; }
    bool doesRequireSelfRef() const { return requires_selfref; }

    void isConstructor(bool val) { is_constructor=val; }
    bool isConstructor() const { return is_constructor; }

    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return args;}
    std::vector<std::unique_ptr<VariableDefAST>>& getModifyableArgs() {return args;}
};

// ExternAST - Class for extern functions which are defined in some other language like C
class ExternAST : public FunctionBaseAST
{
    std::unique_ptr<PrototypeAST> proto;
public:
    int asttype;

    ExternAST(std::unique_ptr<PrototypeAST> prototype)
    : FunctionBaseAST(prototype->getReturnType()), 
    asttype(ast_extern), proto(std::move(prototype))
    {}

    proto::IName const& getIName()    const { return proto->getIName(); }
    std::string const getName()       const { return proto->getName(); }
    
    void setName(std::string const& name) { proto->setName(name); }
    void setName(proto::IName const& name) { proto->setName(name); } 

    VToken* const getNameToken() const { return proto->getNameToken(); }
    std::unique_ptr<VToken> moveNameToken() { return proto->moveNameToken(); }
    
    bool is_extern() const {return true;}
    bool is_type_null() const {return return_type==nullptr;}

    PrototypeAST* const getProto() const {return proto.get();}
    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const {return proto->getArgs();}

    void doesRequireSelfRef(bool val) { }
    bool doesRequireSelfRef() const { return false; }
};

// FunctionAST - Class for functions which can be called by the user
class FunctionAST : public FunctionBaseAST
{
    std::unique_ptr<PrototypeAST> proto;
    std::vector<std::unique_ptr<ExprAST>> statements;
    std::vector<ReturnExprAST*> return_stms;
    std::unordered_map<std::string, VariableDefAST*> locals;
    std::unordered_map<std::string, unsigned int> arg_indxs;
    bool requires_selfref;
    bool is_constructor;
public:
    int asttype;
    
    FunctionAST(std::unique_ptr<PrototypeAST> prototype, std::vector<std::unique_ptr<ExprAST>> stmts, bool requires_selfref=false, bool is_constructor=false)
    : FunctionBaseAST(prototype->getReturnType()),
    asttype(ast_function), proto(std::move(prototype)), statements(std::move(stmts))
    {
    }

    // Getter Functions
    PrototypeAST*                                const getProto() const { return proto.get(); }
    std::vector<std::unique_ptr<VariableDefAST>> const& getArgs() const { return proto->getArgs(); }
    std::vector<std::unique_ptr<VariableDefAST>>& getModifyableArgs()   { return proto->getModifyableArgs(); }
    std::vector<std::unique_ptr<ExprAST>>        const& getBody() const { return statements; }

    proto::IName const& getIName()    const { return proto->getIName(); }
    std::string const getName()       const { return proto->getName(); }

    void setName(std::string const& name) { proto->setName(name); }
    void setName(proto::IName const& name) { proto->setName(name); } 

    VToken* const getNameToken()      const { return proto->getNameToken(); }
    std::unique_ptr<VToken> moveNameToken() { return proto->moveNameToken(); }

    void setReturnType(std::unique_ptr<types::Base> type) { proto->setReturnType(types::copyType(type.get())); this->return_type=std::move(type); }

    // Block-based Functions
    void insertStatement(std::unique_ptr<ExprAST> statement) 
    { statements.insert( statements.begin(), std::move(statement) ); }

    // Variable-based Functions
    bool isVariableDefined(std::string const& name)            const { return locals.count(name)>0; }
    VariableDefAST* const getVariable(std::string const& name) const { return locals.at(name); }
    std::unordered_map<std::string, VariableDefAST*> const& getLocals() const { return locals; }

    // Return statement functions
    std::vector<ReturnExprAST*> const& getReturnStatements() const { return return_stms; }
    void addReturnStatement(ReturnExprAST* ret) { return_stms.push_back(ret); }

    void addVariable(VariableDefAST* const var) { if(var->isArgument()) arg_indxs[var->getName()] = arg_indxs.size(); locals[var->getName()] = var;}
    void addVariables(std::vector<VariableDefAST*> const& vars, bool are_args=false) 
    {
        if(are_args)
            arg_indxs.reserve(arg_indxs.size()+vars.size());
        locals.reserve(locals.size()+vars.size());

        for(auto const& var : vars)
            addVariable(var);
    }
    unsigned int getArgumentIndex(std::string const& name) { return arg_indxs[name]; }

    void doesRequireSelfRef(bool val) { proto->doesRequireSelfRef(val); }
    bool doesRequireSelfRef() const { return proto->doesRequireSelfRef(); }

    void isConstructor(bool val) { proto->isConstructor(val); }
    bool isConstructor() const { return proto->isConstructor(); }
};

class ReturnExprAST : public ExprAST
{
    std::unique_ptr<ExprAST> expr;
    proto::IName func_name;
public:
    ReturnExprAST(std::unique_ptr<ExprAST> expr) : expr(std::move(expr)), ExprAST("",ast_return), 
    func_name("")
    {}

    void setName(std::string name) {func_name = name;}
    void setName(proto::IName const& name) {func_name = name;}
    std::string const& getName() const {return func_name.get();}
    ExprAST* const getValue() const {return expr.get();}
    std::unique_ptr<ExprAST> const moveValue() { return std::move(expr); }
    void setValue(std::unique_ptr<ExprAST> t) { expr=std::move(t); }
};

}