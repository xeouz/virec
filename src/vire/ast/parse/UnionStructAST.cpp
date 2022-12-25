#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include <memory>
#include <vector>
#include <map>
#include <memory>

namespace vire
{

class FunctionAST;

typedef std::unordered_map<proto::IName, std::unique_ptr<ExprAST>, std::hash<proto::IName>, std::equal_to<proto::IName>> INameExprMap;
typedef std::unordered_map<proto::IName, int, std::hash<proto::IName>, std::equal_to<proto::IName>> INameIntMap;

class TypeAST : public ExprAST
{
    INameExprMap members;
    INameIntMap members_indx;
    proto::IName name;
    std::unique_ptr<VToken> name_token;
public:
    TypeAST(INameExprMap members, std::unique_ptr<VToken> name, int asttype=ast_type)
    : members(std::move(members)), members_indx(INameIntMap()), name(name->value), ExprAST("void", asttype)
    {
        name_token=std::move(name);
        int i=this->members.size()-1;
        for(auto& [iname, ptr] : this->members)
        {
            this->members_indx[iname]=i--;
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

    virtual INameExprMap const& getMembers()
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
    virtual ExprAST* getMember(std::string const& name)
    {
        return getMember(proto::IName(name, ""));
    }
};

class UnionExprAST : public TypeAST
{
public:
    UnionExprAST(INameExprMap members, std::unique_ptr<VToken> name)
    : TypeAST(std::move(members), std::move(name), ast_union)
    {
    }
};

class StructExprAST : public TypeAST
{
    std::unique_ptr<FunctionAST> constructor;
public:
    StructExprAST(INameExprMap members, std::unique_ptr<FunctionAST> constructor, std::unique_ptr<VToken> name)
    : TypeAST(std::move(members), std::move(name), ast_struct), constructor(std::move(constructor))
    {
    }
    StructExprAST(INameExprMap members, std::unique_ptr<VToken> name)
    : TypeAST(std::move(members), std::move(name), ast_struct), constructor(nullptr)
    {
    }

    FunctionAST* getConstructor() { return constructor.get(); }
    void setConstructor(std::unique_ptr<FunctionAST> new_constructor) { constructor=std::move(new_constructor); }
};

}