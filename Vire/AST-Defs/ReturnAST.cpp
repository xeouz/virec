#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>

namespace vire
{

class ReturnExprAST : public ExprAST
{
    std::vector<std::unique_ptr<ExprAST>> Values;
public:
    ReturnExprAST(std::vector<std::unique_ptr<ExprAST>> Values) : Values(std::move(Values)), ExprAST("",ast_return)
    {}

    std::vector<std::unique_ptr<ExprAST>> getValues() 
    {
        std::vector<std::unique_ptr<ExprAST>> ret;

        for(auto& item:Values)
        {
            std::unique_ptr<ExprAST> ast(static_cast<ExprAST*>(item.get()));
            ret.push_back(std::move(ast));
        }
        
        return std::move(ret);
    }
};

}