#pragma once

#include "UnionStructAST.cpp"

#include "ExprAST.cpp"

#include <memory>

namespace vire
{

std::unique_ptr<ExprAST> UnionExprAST::getMember(const std::string& varName)
{
        for(auto& item:Members)
        {
            if(item->asttype==ast_struct)
            {
                std::unique_ptr<StructExprAST> structast(static_cast<StructExprAST*>(item.get()));
                return std::move(structast->getMember(varName));
            }
            else if(item->asttype==ast_union)
            {
                std::unique_ptr<UnionExprAST> unionast(static_cast<UnionExprAST*>(item.get()));
                return std::move(unionast->getMember(varName));
            }
            else if(item->asttype==ast_typedvar)
            {
                std::unique_ptr<TypedVarAST> varast(static_cast<TypedVarAST*>(item.get()));
                if(varast->getName() == varName)
                    return std::move(varast);
            }
        }
        return nullptr;
}

std::unique_ptr<ExprAST> StructExprAST::getMember(const std::string& varName)
{
        for(auto& item:Members)
        {
            if(item->asttype==ast_struct)
            {
                std::unique_ptr<StructExprAST> structast(static_cast<StructExprAST*>(item.get()));
                return std::move(structast->getMember(varName));
            }
            else if(item->asttype==ast_union)
            {
                std::unique_ptr<UnionExprAST> unionast(static_cast<UnionExprAST*>(item.get()));
                return std::move(unionast->getMember(varName));
            }
            else if(item->asttype==ast_typedvar)
            {
                std::unique_ptr<TypedVarAST> varast(static_cast<TypedVarAST*>(item.get()));
                if(varast->getName() == varName)
                    return std::move(varast);
            }
        }
        return nullptr;
}

}