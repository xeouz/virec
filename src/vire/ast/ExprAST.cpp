#pragma once

#include "../includes.hpp"

#include <string>
#include <memory>
#include <vector>

#include "../includes.hpp"
#include __VIRE_LEX_PATH

namespace vire
{

class ExprAST{
protected:
    std::string type;
    std::unique_ptr<Viretoken> token;
public:
    int asttype;
    ExprAST(const std::string& type, int asttype, std::unique_ptr<Viretoken> token=nullptr)
    : type(type), asttype(asttype), token(std::move(token)) {}

    virtual ~ExprAST() {}
    virtual std::string getType() const {return type;}
    virtual void setType(const std::string& newtype) {type=newtype;}

    virtual const std::size_t& getLine()    const {return token->line;}
    virtual const std::size_t& getCharpos() const {return token->charpos;} 
    virtual void setToken(std::unique_ptr<Viretoken> token) {this->token.reset();this->token=std::move(token);}
};

}