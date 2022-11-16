#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>

namespace vire
{

class TypeAST : public ExprAST
{
    std::unordered_map<proto::IName, std::unique_ptr<ExprAST>> members;
    std::unordered_map<proto::IName, int> members_indx;
    proto::IName name;
    std::unique_ptr<VToken> name_token;
public:
    TypeAST(std::unordered_map<proto::IName, std::unique_ptr<ExprAST>> members, std::unique_ptr<VToken> name, int asttype=ast_type)
    : members(std::move(members)), members_indx(std::unordered_map<proto::IName, int>()), name(name->value), ExprAST("void", asttype)
    {
        name_token=std::move(name);
        int i=this->members.size()-1;
        for(auto& [str, ptr] : this->members)
        {
            this->members_indx[str]=i--;
        }
    }

    virtual std::string const& getName() const
    {
        return name.get();
    }
    virtual proto::IName const& getIName() const
    {
        return name;
    }
    virtual void setName(std::string new_name)
    {
        name.setName(new_name);
    }

    virtual std::unordered_map<proto::IName, std::unique_ptr<ExprAST>> const& getMembers()
    {
        return members;
    }
    virtual std::vector<ExprAST*> const getMembersValues() const
    {
        std::vector<ExprAST*> values;
        values.reserve(members.size());

        for(auto& [str,ptr]:members)
        {
            values.push_back(ptr.get());
        }

        return values;
    }
    virtual int const getMemberIndex(proto::IName const& name)
    {
        return members_indx.at(name);
    }

    virtual bool isMember(proto::IName const& name)
    {
        if(members.count(name)>0)
        {
            return true;
        }

        return false;
    }
    virtual ExprAST* getMember(proto::IName const& name)
    {
        return members.at(name).get();
    }
};

class UnionExprAST : public TypeAST
{
public:
    UnionExprAST(std::unordered_map<proto::IName, std::unique_ptr<ExprAST>> members, std::unique_ptr<VToken> name)
    : TypeAST(std::move(members), std::move(name), ast_union)
    {
    }
};

class StructExprAST : public TypeAST
{
public:
    StructExprAST(std::unordered_map<proto::IName, std::unique_ptr<ExprAST>> members, std::unique_ptr<VToken> name)
    : TypeAST(std::move(members), std::move(name), ast_struct)
    {
    }
};

}