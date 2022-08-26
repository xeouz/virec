#pragma once

#include <string>
#include <memory>
#include <vector>

#include "vire/lex/include.hpp"
#include "vire/ast/types/type.hpp"

namespace vire
{

class ExprAST
{
protected:
    std::unique_ptr<types::Base> type;
    std::unique_ptr<Viretoken> token;
public:
    int asttype;
    ExprAST(const std::string& type, int asttype, std::unique_ptr<Viretoken> token=nullptr)
    : asttype(asttype), token(std::move(token)), type(types::construct(type))
    {}

    ExprAST(std::unique_ptr<types::Base> type, int asttype, std::unique_ptr<Viretoken> token=nullptr)
    : asttype(asttype), token(std::move(token)), type(std::move(type))
    {}
   
    virtual types::Base* getType() const 
    {
        return type.get(); 
    }

    virtual void setType(std::unique_ptr<types::Base> t) 
    {
        if(type->getType()==types::TypeNames::Void)
        {
            auto* voidty=(types::Void*)t.get();
            if(types::isTypeinMap(voidty->getName()))
            {
                t=types::construct(voidty->getName(), true);
            }
        }
        this->type=std::move(t);
    }
    virtual void setType(std::string const& newtype) 
    {
        setType(types::construct(newtype)); 
    }
    virtual void setType(types::Base* t)
    {
        setType(std::unique_ptr<types::Base>(t));
    }

    virtual const std::size_t& getLine()    const 
    {
        return token->line;
    }
    virtual const std::size_t& getCharpos() const 
    {
        return token->charpos;
    } 
    virtual void setToken(std::unique_ptr<Viretoken> token) 
    {
        this->token.reset(); this->token=std::move(token);
    }
};

}