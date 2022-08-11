#pragma once

#include <string>
#include <memory>

#include "ASTType.hpp"
#include "ExprAST.cpp"

namespace vire
{
// VariableExprAST - Class for referencing a variable, eg - `myvar`
class VariableExprAST : public ExprAST
{
    std::string name;
public:
    VariableExprAST(std::unique_ptr<Viretoken> name) : name(name->value), ExprAST("",ast_var) 
    {setToken(std::move(name));} 

    std::string const& getName() const {return name;}
    Viretoken* const getToken() const {return token.get();}
    std::unique_ptr<Viretoken> moveToken() {return std::move(token);}
};

class VariableAssignAST: public ExprAST
{
    std::string name;
    std::unique_ptr<ExprAST> value;
public: 
    VariableAssignAST(std::unique_ptr<Viretoken> name, std::unique_ptr<ExprAST> value)
    : name(name->value), value(std::move(value)), ExprAST("void",ast_varassign) {setToken(std::move(name));}

    std::string const& getName() const {return name;}
    ExprAST* const getValue() const {return value.get();}
};

class VariableArrayAccessAST: public ExprAST
{
    std::string name;
    std::unique_ptr<ExprAST> indx;
public:
    VariableArrayAccessAST(std::unique_ptr<Viretoken> name, std::unique_ptr<ExprAST> indx)
    : name(name->value), indx(std::move(indx)), ExprAST("void",ast_array_access) {setToken(std::move(name));}
    
    std::string const& getName() const {return name;}
    ExprAST* const getIndex() const {return indx.get();}
};

class VariableDefAST : public ExprAST
{
    std::string name;
    std::unique_ptr<ExprAST> value;
    bool is_const, is_let, is_array;
    bool use_value_type;
public:
    VariableDefAST(std::unique_ptr<Viretoken> name, std::unique_ptr<types::Base> type, std::unique_ptr<ExprAST> value,
    bool is_const=false, bool is_let=false)
    : name(name->value),value(std::move(value)),ExprAST(std::move(type),ast_vardef), 
    is_const(is_const),is_let(is_let), use_value_type(false)
    { setToken(std::move(name)); }

    std::string const& getName() const {return name;}
    const bool& isConst() const {return is_const;}
    const bool& isLet() const {return is_let;}

    types::Base* getType() const
    {
        if(type==nullptr || use_value_type)
        {
            return value->getType();
        }

        return type.get();
    }
    void setType(std::unique_ptr<types::Base> type) 
    {
        this->type=nullptr;
        value->setType(std::move(type));
    }
    void setType(types::Base* t)
    {
        value->setType(t);
    }

    ExprAST* const getValue() const {return value.get();}
    std::unique_ptr<ExprAST> moveValue() {return std::move(value);}
    void setValue(std::unique_ptr<ExprAST> value) {this->value=std::move(value);}

    void setUseValueType(bool use_value_type) {this->use_value_type=use_value_type;}
};

class TypedVarAST : public ExprAST
{
    std::unique_ptr<Viretoken> name;
public:
    TypedVarAST(std::unique_ptr<Viretoken> name, std::unique_ptr<Viretoken> Type) 
    : name(std::move(name)), ExprAST(Type->value,ast_typedvar)
    {setToken(std::move(Type));}

    std::string const& getName() const {return name->value;}
};

class VariableIncrDecrAST : public ExprAST
{
    std::string name;
    bool isincr, ispre;
public:
    VariableIncrDecrAST(std::unique_ptr<Viretoken> name, bool isincr, bool ispre)
    : name(name->value), ExprAST("void",ast_varincrdecr), isincr(isincr), ispre(ispre)
    {setToken(std::move(name));}
    
    std::string const& getName() const {return name;}
    bool isIncr() const {return isincr;}
    bool isPre() const {return ispre;}
};

}