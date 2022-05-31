#pragma once

#include <string>

namespace vire
{
class ExprAST{
protected:
    std::string type;
public:
    int asttype;
    ExprAST(const std::string& type, int asttype) : type(type), asttype(asttype) {}

    virtual ~ExprAST() {}
    virtual std::string getType() const {return type;}
};
}