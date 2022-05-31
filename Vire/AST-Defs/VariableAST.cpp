#pragma once

#include "../Lex/token.hpp"

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
    VariableExprAST(const std::string& Name) : Name(Name), ExprAST("",ast_var) {}
    void setType(const std::string& newtype) {this->type=std::string(newtype);}

    const std::string& getName() const {return Name;}
};

class VariableAssignAST: public ExprAST
{
    std::string Name;
    std::unique_ptr<ExprAST> Value;
public: 
    VariableAssignAST(const std::string& Name, std::unique_ptr<ExprAST> Value)
    : Name(Name), Value(std::move(Value)), ExprAST("void",ast_varassign) {}
};

class VariableDefAST : public ExprAST
{
    std::string Name;
    std::unique_ptr<ExprAST> Value;
    unsigned char isconst, islet, isarr;
    unsigned int arr_size;
public:
    VariableDefAST(const std::string& Name, const std::string& type, std::unique_ptr<ExprAST> Value,
    unsigned char isconst=0, unsigned char islet=0, unsigned char isarr=0, int arr_size=0)
    : Name(Name),Value(std::move(Value)),ExprAST(type,ast_vardef),
    isconst(isconst),islet(islet),isarr(isarr),arr_size(arr_size) {}

    std::string getName() const {return Name;}
    const unsigned char& isConst() const {return isconst;}
    const unsigned char& isLet() const {return islet;}
    const unsigned char& isArr() const {return isarr;}
    const unsigned int& getArrSize() const {return arr_size;}
};

class TypedVarAST : public ExprAST
{
    std::string Name;
public:
    TypedVarAST(std::string Name, std::string Type) : Name(Name), ExprAST(Type, ast_typedvar)
    {}

    const std::string& getName() const {return Name;}
};

}