#include "analyzer.hpp"

#include "../AST-Parse/Include.hpp"
#include "../AST-Defs/Include.hpp"
#include "../Error-Builder/Include.hpp"

#include <string>
#include <memory>
#include <vector>
#include <algorithm>

namespace vire
{
    bool VAnalyzer::isVarDefined(const std::string& name)
    {
        if(!variables.empty())
        {
            for(int i=0; i<variables.size(); i++)
            {
                if(variables[i]->getName()==name)
                {
                    return true;
                }
            }
        }
        return false;
    }
    bool VAnalyzer::addVar(std::unique_ptr<VariableDefAST> var)
    {
        if(!isVarDefined(var->getName()))
        {
            variables.push_back(std::move(var));
            return true;
        }
        return false;
    }

    const std::unique_ptr<VariableDefAST>& VAnalyzer::getVar(const std::string& name)
    {
        if(!variables.empty())
        {
            for(int i=0; i<variables.size(); i++)
            {
                if(variables[i]->getName()==name)
                {
                    return variables[i];
                }
            }
        }
        return variables[0]; // never reach this case, we are assured that the variable is defined, im a bad programmer
    }
 
    std::string VAnalyzer::getType(const std::unique_ptr<ExprAST>& expr)
    {
        switch (expr->asttype)
        {
            case ast_int: return "int";
            case ast_float: return "float";
            case ast_double: return "double";
            case ast_str: return "str";
            case ast_char: return "char";

            default: return "";
        }
    }
    std::string VAnalyzer::getType(const std::unique_ptr<VariableExprAST>& var)
    {
        if(isVarDefined(var->getName()))
        {
            return getVar(var->getName())->getType();
        }
        return "";
    }
    std::string VAnalyzer::getType(const std::unique_ptr<ArrayExprAST>& array)
    {
        const auto& vec = array->getElements();
        std::string type=getType(vec[0]);
        for(int i=0; i<vec.size(); ++i)
        {
            std::string newType=getType(vec[i]);
            if(type!=newType)
            {
                return "any";
            }
        }

        return type;
    }

    // Verification Functions
    bool VAnalyzer::verifyVar(std::unique_ptr<VariableExprAST> var)
    {
        // Check if it is defined
        if(!isVarDefined(var->getName()))
        {
            // Var is not defined
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyVarDef(std::unique_ptr<VariableDefAST> var)
    {
        if(!isVarDefined(var->getName()))
        {
            bool isvar=!(var->isLet() || var->isConst());
            if(!isvar)
            {
                if(var->getValue() == nullptr && var->getType() == "auto")
                {
                    // Requires a variable for definiton
                    unsigned char islet = var->isLet() ? 1 : 0;
                    builder->addError<errors::analyze_requires_type>
                    (this->code, islet, var->getName(), var->getLine(), var->getCharpos());
                    return false;
                }
            }
 
            std::string type=var->getType();
            std::string value_type;
            bool is_array=var->isArr() || var->getValue()->asttype==ast_array;
            bool is_auto=type=="auto" || type=="";

            if(is_auto)
            {
                if(isvar)
                {
                    std::cout << "any type not implement yet" << std::endl;
                    type=getType(var->getValue());
                }
                else
                {
                    type=getType(var->getValue());
                }
            }
            
            if(is_array)
            {
                auto arr=cast_static<ArrayExprAST>(var->moveValue());
                value_type=getType(arr);
                var->setValue(std::move(arr));
                if(is_auto) type=value_type;
            }
            else
            {
                value_type=getType(var->getValue());
            }

            if(value_type != type)
            {
                // Type mismatch
                std::cout << "Type mismatch, types are: " << value_type << " and " << type << std::endl;
                return false;
            }

            type=value_type;
            var->setType(type);
            addVar(std::move(var));
        }
        // Variable is already defined
        return true;
    }
    bool VAnalyzer::verifyTypedVar(std::unique_ptr<TypedVarAST> var)
    {
        return true;
    }
    bool VAnalyzer::verifyVarAssign(std::unique_ptr<VariableAssignAST> assign)
    {
        if(!isVarDefined(assign->getName()))
        {
            // Var is not defined
            return false;
        }

        const std::unique_ptr<VariableDefAST>& var=getVar(assign->getName());
        if(var->isConst())
        {
            // Var is const
            std::cout << "Var is const" << std::endl;
            return false;
        }

        auto assign_type=getType(assign->getValue());
        if(var->getType() != assign_type)
        {
            // Type mismatch
            std::cout << "Type mismatch in assignment, types are: " << var->getType() << " and " << assign_type << std::endl;
            return false;
        }

        return true;
    }

/*
    bool VAnalyzer::verifyExpr(std::unique_ptr<ExprAST> expr)
    {
        switch (expr->asttype)
        {
            case ast_int: return verifyInt(cast_static<IntExprAST>(std::move(expr)));
            case ast_float: return verifyFloat(cast_static<FloatExprAST>(std::move(expr)));
            case ast_double: return verifyDouble(cast_static<DoubleExprAST>(std::move(expr)));
            case ast_str: return verifyStr(cast_static<StrExprAST>(std::move(expr)));
            case ast_char: return verifyChar(cast_static<CharExprAST>(std::move(expr)));
            case ast_var: return verifyVar(cast_static<VariableExprAST>(std::move(expr)));
            case ast_vardef: return verifyVarDef(cast_static<VariableDefAST>(std::move(expr)));
            case ast_typedvar: return verifyTypedVar(cast_static<TypedVarAST>(std::move(expr)));
            case ast_varassign: return verifyVarAssign(cast_static<VariableAssignAST>(std::move(expr)));

            default: return false;
        }
    }
*/
    bool VAnalyzer::verifyArray(std::unique_ptr<ArrayExprAST> array)
    {
        //const std::vector<std::unique_ptr<ExprAST>>& values=array->getElements();
        //bool is_valid=true;
        /*for(int i=0; i<values.size(); i++)
        {
            if(!verifyExpr(values[i]))
            {
                is_valid=false;
            }
        }*/
        return true;
    }
}