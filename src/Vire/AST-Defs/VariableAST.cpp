#pragma once

#include "../Lex/token.hpp"
#include "../Lex/token.cpp"

#include <string>
#include <memory>

#include "ASTType.hpp"
#include "ExprAST.cpp"

namespace vire
{
// VariableExprAST - Class for referencing a variable, eg - `myvar`
class VariableExprAST : public ExprAST
{
    std::string Name;
public:
    VariableExprAST(std::unique_ptr<Viretoken> Name) : Name(Name->value), ExprAST("",ast_var) 
    {setToken(std::move(Name));} 

    const std::string& getName() const {return Name;}
    const std::unique_ptr<Viretoken>& getToken() const {return token;}
    std::unique_ptr<Viretoken> moveToken() {return std::move(token);}
};

class VariableAssignAST: public ExprAST
{
    std::string Name;
    std::unique_ptr<ExprAST> Value;
public: 
    VariableAssignAST(std::unique_ptr<Viretoken> Name, std::unique_ptr<ExprAST> Value)
    : Name(Name->value), Value(std::move(Value)), ExprAST("void",ast_varassign) {setToken(std::move(Name));}
};

class VariableDefAST : public ExprAST
{
    std::string Name;
    std::unique_ptr<ExprAST> Value;
    unsigned char isconst, islet, isarr;
    unsigned int arr_size;
public:
    VariableDefAST(std::unique_ptr<Viretoken> Name, const std::string& type, std::unique_ptr<ExprAST> Value,
    unsigned char isconst=0, unsigned char islet=0, unsigned char isarr=0, int arr_size=0)
    : Name(Name->value),Value(std::move(Value)),ExprAST(type,ast_vardef), 
    isconst(isconst),islet(islet),isarr(isarr),arr_size(arr_size) {setToken(std::move(Name));}

    const std::string& getName() const {return Name;}
    const unsigned char& isConst() const {return isconst;}
    const unsigned char& isLet() const {return islet;}
    const unsigned char& isArr() const {return isarr;}
    const unsigned int& getArrSize() const {return arr_size;}

    const std::unique_ptr<ExprAST>& getValue() const {return Value;}
};

class TypedVarAST : public ExprAST
{
    std::unique_ptr<Viretoken> Name;
public:
    TypedVarAST(std::unique_ptr<Viretoken> Name, std::unique_ptr<Viretoken> Type) 
    : Name(std::move(Name)), ExprAST(Type->value,ast_typedvar)
    {setToken(std::move(Type));}

    const std::string& getName() const {return Name->value;}
};

}