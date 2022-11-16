#pragma once

#include <string>
#include <memory>

#include "ASTType.hpp"
#include "ExprAST.cpp"

namespace vire
{
    
// VariableArrayAccessAST - Class for array access, eg - `foo[bar]`
class VariableArrayAccessAST: public ExprAST
{
    std::unique_ptr<ExprAST> expr;
    std::vector<std::unique_ptr<ExprAST>> indices;
public:
    VariableArrayAccessAST(std::unique_ptr<ExprAST> expr, std::vector<std::unique_ptr<ExprAST>> indx)
    : expr(std::move(expr)), indices(std::move(indx)), ExprAST("void",ast_array_access) 
    {
    }
    
    ExprAST* const getExpr() const 
    {
        return expr.get();
    }
    std::vector<std::unique_ptr<ExprAST>> const& getIndices() const 
    {
        return indices;
    }
};

class VariableAssignAST: public ExprAST
{
    std::unique_ptr<ExprAST> lhs;
    std::unique_ptr<ExprAST> rhs;
    std::unique_ptr<VToken> shorthand_op;
public:
    bool is_shorthand;

    VariableAssignAST(std::unique_ptr<ExprAST> lhs, std::unique_ptr<ExprAST> rhs, std::unique_ptr<VToken> _shorthand_op=nullptr)
    : lhs(std::move(lhs)), rhs(std::move(rhs)), ExprAST("void",ast_varassign)
    {
        if(_shorthand_op)
        {
            is_shorthand=true;
            shorthand_op=std::move(_shorthand_op);
        }
        else
        {
            is_shorthand=false;
        }
    }

    ExprAST* const getLHS() const
    {
        return lhs.get();
    }
    ExprAST* const getRHS() const
    {
        return rhs.get();
    }
    VToken* const getShorthandOperator()
    {
        return shorthand_op.get();
    }

    std::unique_ptr<ExprAST> const moveRHS()
    {
        return std::move(rhs);
    }
    
    void setRHS(std::unique_ptr<ExprAST> _rhs)
    {
        rhs=std::move(_rhs);
    }
};

class VariableDefAST : public ExprAST
{
    proto::IName name;
    std::unique_ptr<ExprAST> value;
    bool is_const, is_let, is_array;
    bool use_value_type;
public:
    VariableDefAST(std::unique_ptr<VToken> name, std::unique_ptr<types::Base> type, std::unique_ptr<ExprAST> value,
    bool is_const=false, bool is_let=false)
    : name(name->value), value(std::move(value)), ExprAST(std::move(type),ast_vardef), 
    is_const(is_const),is_let(is_let), use_value_type(false)
    {
        setToken(std::move(name));
    }

    std::string const& getName() const {return name.get();}
    proto::IName const& getIName() const {return name;}
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

class CastExprAST : public ExprAST
{
    std::unique_ptr<ExprAST> expr;
    std::unique_ptr<types::Base> source_type;
    bool is_non_user_defined;
public:
    CastExprAST(std::unique_ptr<ExprAST> expr, std::unique_ptr<types::Base> type, bool is_non_user_defined=false)
    : expr(std::move(expr)), source_type(std::move(type)), ExprAST("void",ast_cast), is_non_user_defined(is_non_user_defined)
    {}

    ExprAST* const getExpr() const 
    {
        return expr.get();
    }
    types::Base* getType() const 
    {
        return expr->getType();
    }
    types::Base* getDestType() const 
    {
        return source_type.get();
    }
    types::Base* getSourceType() const 
    {
        return expr->getType();
    }
    bool isNonUserDefined() const 
    {
        return is_non_user_defined;
    }

    void setDestType(std::unique_ptr<types::Base> type) 
    {
        source_type=std::move(type);
    }
    void setSourceType(std::unique_ptr<types::Base> type) 
    {
        expr->setType(std::move(type));
    }
};

}