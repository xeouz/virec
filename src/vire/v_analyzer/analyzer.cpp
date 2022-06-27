#include "analyzer.hpp"
#include <string>
#include <memory>
#include <vector>
#include <algorithm>

namespace vire
{
    bool VAnalyzer::isVarDefined(const std::string& name)
    {
        const auto& variables=(*current_function)->getLocals();
        if(!variables.empty())
        {
            for(int i=0; i<variables.size(); ++i)
            {
                if(variables[i]->getName()==name)
                {
                    return true;
                }
            }
        }
        return false;
    }
    bool VAnalyzer::isFuncDefined(const std::string& name)
    {
        const auto& functions=current_code->getFunctions();
        if(!functions.empty())
        {
            for(int i=0; i<functions.size(); ++i)
            {
                if(functions[i]->getName()==name)
                {
                    return true;
                }
            }
        }
        return false;
    }
    bool VAnalyzer::isClassDefined(const std::string& name)
    {
        const auto& classes=current_code->getClasses();
        if(!classes.empty())
        {
            for(int i=0; i<classes.size(); ++i)
            {
                if(classes[i]->getName()==name)
                {
                    return true;
                }
            }
        }
        return false;
    }
    bool VAnalyzer::isStdFunc(const std::string& name)
    {
        if(name=="echo")
        {
            return true;
        }
        return false;
    }
    
    bool VAnalyzer::addVar(std::unique_ptr<VariableDefAST> var)
    {
        if(!isVarDefined(var->getName()))
        {
            (*current_function)->addLocal(std::move(var));
            return true;
        }
        return false;
    }
    bool VAnalyzer::addProto(std::unique_ptr<PrototypeAST> proto)
    {
        if(!isFuncDefined(proto->getName()))
        {
            current_code->addFunction(std::move(proto));
            return true;
        }
        return false;
    }
    bool VAnalyzer::addFunc(std::unique_ptr<FunctionAST> func)
    {
        if(!isFuncDefined(func->getName()))
        {
            current_code->addFunction(std::move(func));
            return true;
        }
        return false;
    }
    bool VAnalyzer::addExtern(std::unique_ptr<ExternAST> extern_)
    {
        if(!isFuncDefined(extern_->getName()))
        {
            current_code->addFunction(std::move(extern_));
            return true;
        }
        return false;
    }

    const std::unique_ptr<FunctionBaseAST>& VAnalyzer::getFunc(const std::string& name)
    {
        const auto& functions=current_code->getFunctions();
        for(int i=0; i<functions.size(); i++)
        {
            if(functions[i]->getName()==name)
            {
                return functions[i];
            }
        }
        return functions[0];
    }
    unsigned int VAnalyzer::getFuncArgCount(const std::string& name) {return getFunc(name)->getArgs().size();}

    const std::unique_ptr<VariableDefAST>& VAnalyzer::getVar(const std::string& name)
    {
        const auto& variables=(*current_function)->getLocals();
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
    bool VAnalyzer::verifyVar(const std::unique_ptr<VariableExprAST>& var)
    {
        // Check if it is defined
        if(!isVarDefined(var->getName()))
        {
            // Var is not defined
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyVarDef(const std::unique_ptr<VariableDefAST>& var)
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
        }
        // Variable is already defined
        return true;
    }
    bool VAnalyzer::verifyTypedVar(const std::unique_ptr<TypedVarAST>& var)
    {
        return true;
    }
    bool VAnalyzer::verifyVarAssign(const std::unique_ptr<VariableAssignAST>& assign)
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

    bool VAnalyzer::verifyInt(const std::unique_ptr<IntExprAST>& int_) { return true; }
    bool VAnalyzer::verifyFloat(const std::unique_ptr<FloatExprAST>& float_) { return true; }
    bool VAnalyzer::verifyDouble(const std::unique_ptr<DoubleExprAST>& double_) { return true; }
    bool VAnalyzer::verifyChar(const std::unique_ptr<CharExprAST>& char_) { return true; }
    bool VAnalyzer::verifyStr(const std::unique_ptr<StrExprAST>& str) { return true; }
    bool VAnalyzer::verifyArray(const std::unique_ptr<ArrayExprAST>& array)
    {
        const auto& elems=array->getElements();

        for(const auto& elem : elems)
        {
            if(!verifyExpr(elem))
            {
                // Element is not valid
                return false;
            }
        }
        return true;
    }

    bool VAnalyzer::verifyFor(const std::unique_ptr<ForExprAST>& for_)
    { 
        const auto& init=for_->getInit();
        const auto& cond=for_->getCond();
        const auto& incr=for_->getIncr();
        if(!(init->asttype==ast_var || init->asttype==ast_varassign || init->asttype==ast_vardef))
        {
            // Init is not a variable definition
            return false;
        }
        if(!(cond->asttype==ast_var || cond->asttype==ast_unop || cond->asttype==ast_binop))
        {
            // Cond is not a boolean expression
            return false;
        }
        if(!(incr->asttype==ast_var || incr->asttype==ast_varassign))
        {
            // Incr is not a step operation
            return false;
        }   

        if(!verifyExpr(init))
        {
            // Init is not valid
            return false;
        }
        if(!verifyExpr(cond))
        {
            // Cond is not valid
            return false;
        }
        if(!verifyExpr(incr))
        {
            // Incr is not valid
            return false;
        }

        if(!verifyBlock(for_->getBody()))
        {
            // Block is not valid
            return false;
        }
        
        return true;
    }
    bool VAnalyzer::verifyWhile(const std::unique_ptr<WhileExprAST>& while_)
    {
        const auto& cond=while_->getCond();

        if(cond->asttype!=ast_var && cond->asttype!=ast_unop && cond->asttype!=ast_binop)
        {
            // Cond is not a boolean expression
            return false;
        }

        if(!verifyExpr(cond))
        {
            // Cond is not valid
            return false;
        }
        if(!verifyBlock(while_->getBody()))
        {
            // Block is not valid
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyBreak(const std::unique_ptr<BreakExprAST>& break_) { return true; }
    bool VAnalyzer::verifyContinue(const std::unique_ptr<ContinueExprAST>& continue_) { return true; }    

    bool VAnalyzer::verifyCall(const std::unique_ptr<CallExprAST>& call)
    {
        auto name=call->getName();

        if(isStdFunc(name))
        {
            return true;
        }
        else if(!isFuncDefined(name))
        {
            // Function is not defined
            return false;
        }

        const auto& args=call->getArgs();
        const auto& func_args=getFunc(name)->getArgs();

        if(args.size() != func_args.size())
        {
            // Argument count mismatch
            return false;
        }

        for(unsigned int i=0; i<args.size(); ++i)
        {
            const auto& arg=args[i];
            if(!verifyExpr(arg))
            {
                // Argument is not valid
                return false;
            }

            if(func_args[i]->getType() != getType(arg))
            {
                // Argument type mismatch
                return false;
            }
        }

        return true;
    }

    bool VAnalyzer::verifyReturn(const std::unique_ptr<ReturnExprAST>& ret)
    {
        const auto& func=getFunc(ret->getName());
        const auto& ret_type=func->getType();

        if(ret_type!=getType(ret->getValues()[0]))
        {
            // Type mismatch
            return false;
        }

        return true;
    }

    bool VAnalyzer::verifyBinop(const std::unique_ptr<BinaryExprAST>& binop)
    {
        const auto& left=binop->getLHS();
        const auto& right=binop->getRHS();

        if(!verifyExpr(left))
        {
            // Left is not valid
            return false;
        }
        if(!verifyExpr(right))
        {
            // Right is not valid
            return false;
        }
        
        return true;
    }
    bool VAnalyzer::verifyUnop(const std::unique_ptr<UnaryExprAST>& unop)
    {
        const auto& expr=unop->getExpr();

        if(!verifyExpr(expr))
        {
            // Expr is not valid
            return false;
        }
        
        return true;
    }

    bool VAnalyzer::verifyPrototype(const std::unique_ptr<PrototypeAST>& proto)
    {
        if(isFuncDefined(proto->getName()))
        {
            // Function is already defined
            return false;
        }

        if(proto->is_type_null())
        {
            if(proto->getName()!="main")
            {
                // Function is not main
                return false;
            }
        }
        else if(proto->getType()=="auto" || proto->getType()=="any")
        {
            // Type is not valid
            return false;
        }

        bool is_valid=true;
        const auto& args=proto->getArgs();
        for(const auto& arg : args)
        {
            if(arg->getType() == "auto" || arg->getType() == "any")
            {
                // Type is not valid
                is_valid=false;
            }

            if(!verifyVarDef(arg))
            {
                // Argument is not valid
                is_valid=false;
            }
        }

        return is_valid;
    }
    bool VAnalyzer::verifyProto(const std::unique_ptr<PrototypeAST>& proto)
    {
        if(!verifyPrototype(proto))
        {
            // Prototype is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyExtern(const std::unique_ptr<ExternAST>& extern_)
    {
        if(!verifyPrototype(extern_->getProto()))
        {
            // Prototype is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyFunction(const std::unique_ptr<FunctionAST>& func)
    {
        if(!verifyPrototype(func->getProto()))
        {
            // Prototype is not valid
            return false;
        }

        if(!verifyBlock(func->getBody()))
        {
            // Block is not valid
            return false;
        }

        return true;
    }

    bool VAnalyzer::verifyUnionStructBody(std::vector<std::unique_ptr<ExprAST>> const& body)
    {
        for(const auto& expr: body)
        {
            if(expr->asttype==ast_typedvar)
            {
                if(!verifyTypedVar((const std::unique_ptr<TypedVarAST>&)expr))
                {
                    // TypedVar is not valid
                    return false;
                }
            }
            else if(expr->asttype==ast_struct)
            {
                if(!verifyStruct((const std::unique_ptr<StructExprAST>&)expr))
                {
                    // Struct is not valid
                    return false;
                }
            }
            else if(expr->asttype==ast_union)
            {
                if(!verifyUnion((const std::unique_ptr<UnionExprAST>&)expr))
                {
                    // Union is not valid
                    return false;
                }
            }
        }

        return true;
    }
    bool VAnalyzer::verifyUnion(const std::unique_ptr<UnionExprAST>& union_)
    {
        const auto& members=union_->getMembers();
        
        if(!verifyUnionStructBody(members))
        {
            // Union body is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyStruct(const std::unique_ptr<StructExprAST>& struct_)
    {
        const auto& members=struct_->getMembers();
        
        if(!verifyUnionStructBody(members))
        {
            // Struct body is not valid
            return false;
        }

        return true;
    }

    bool VAnalyzer::verifyIfThen(const std::unique_ptr<IfThenExpr>& if_then)
    {
        const auto& cond=if_then->getCondition();
        const auto& then_block=if_then->getThenBlock();
        
        if(cond->asttype!=ast_var && cond->asttype!=ast_unop && cond->asttype!=ast_binop)
        {
            // Cond is not a boolean expression
            return false;
        }
        if(!verifyExpr(cond))
        {
            // Cond is not valid
            return false;
        }
        if(!verifyBlock(then_block))
        {
            // Then block is not valid
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyIf(const std::unique_ptr<IfExprAST>& if_)
    {
        const auto& cond=if_->getCondition();
        const auto& then_block=if_->getThenBlock();
        
        if(cond->asttype!=ast_var && cond->asttype!=ast_unop && cond->asttype!=ast_binop)
        {
            // Cond is not a boolean expression
            return false;
        }
        if(!verifyExpr(cond))
        {
            // Cond is not valid
            return false;
        }
        if(!verifyBlock(then_block))
        {
            // Then block is not valid
            return false;
        }

        const auto& elif_ladder=if_->getElifLadder();
        unsigned int elif_ladder_size=elif_ladder.size();
        for(unsigned int i=0; i<elif_ladder_size-1; ++i)
        {
            if(!verifyIfThen(elif_ladder[i]))
            {
                // Elif is not valid
                return false;
            }
        }
        if(!verifyBlock(elif_ladder[elif_ladder_size]->getThenBlock()))
        {
            // Else block is not valid
            return false;
        }

        return true;
    }

    bool VAnalyzer::verifyUnsafe(const std::unique_ptr<UnsafeExprAST>& unsafe)
    {
        const auto& block=unsafe->getBody();
        if(!verifyBlock(block))
        {
            // Block is not valid
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyReference(const std::unique_ptr<ReferenceExprAST>& reference)
    {
        const auto& expr=reference->getVar();
        if(!verifyExpr(expr))
        {
            // Expr is not valid
            return false;
        }
        return true;
    }

    bool VAnalyzer::verifyClass(const std::unique_ptr<ClassAST>& cls)
    {
        if(isClassDefined(cls->getName()))
        {
            // Class is already defined
            return false;
        }
        
        const auto& members=cls->getMembers();
        const auto& funcs=cls->getFunctions();

        bool is_valid=true;
        for(const auto& member : members)
        {
            if(!verifyVarDef(member))
            {
                // Member is not valid
                return false;
            }
        }
        for(const auto& func : funcs)
        {
            if(func->is_extern())
            {
                is_valid=false;
            }
            else if(func->is_proto())
            {
                if(!verifyPrototype((std::unique_ptr<PrototypeAST> const&)func))
                {
                    // Prototype is not valid
                    return false;
                }
            }
            else
            {
                const auto& func_cast=(std::unique_ptr<FunctionAST> const&)func;

                if(!verifyPrototype((func_cast->getProto())))
                {
                    // Prototype is not valid
                    return false;
                }
                if(!verifyBlock(func_cast->getBody()))
                {
                    // Block is not valid
                    return false;
                }
            }
        }

        return true;
    }

    bool VAnalyzer::verifyBlock(std::vector<std::unique_ptr<ExprAST>> const& block)
    {
        for(const auto& expr : block)
        {
            if(!verifyExpr(expr))
            {
                // Expr is not valid
                return false;
            }
        }
        return true;
    }

    bool VAnalyzer::verifyCode(std::unique_ptr<CodeAST> code)
    {
        current_code=std::move(code);

        auto classes=current_code->moveClasses();
        auto funcs=current_code->moveFunctions();
        auto union_structs=current_code->moveUnionStructs();
        auto pre_stms=current_code->movePreExecutionStatements();

        bool has_main=false;
        unsigned int main_func_indx=0;
        for(const auto& cls : classes)
        {
            if(!verifyClass(cls))
            {
                // Class is not valid
                return false;
            }
        }
        for(unsigned int it=0; it<funcs.size(); ++it)
        {
            const auto& func=funcs[it];
            if(func->is_extern())
            {
                if(!verifyExtern((std::unique_ptr<ExternAST> const&)func))
                {
                    // Extern is not valid
                    return false;
                }
            }
            else if(func->is_proto())
            {
                if(!verifyPrototype((std::unique_ptr<PrototypeAST> const&)func))
                {
                    // Prototype is not valid
                    return false;
                }
            }
            {
                const auto& func_cast=(std::unique_ptr<FunctionAST> const&)func;

                if(!verifyPrototype(func_cast->getProto()))
                {
                    // Prototype is not valid
                    return false;
                }

                if(!verifyBlock(func_cast->getBody()))
                {
                    // Block is not valid
                    return false;
                }

                if(func_cast->getName()=="main")
                {
                    has_main=true;
                    main_func_indx=it;
                }
            }
        }
        for(const auto& union_struct : union_structs)
        {
            if(union_struct->asttype==ast_union)
            {
                if(!verifyUnion((std::unique_ptr<UnionExprAST> const&)union_struct))
                {
                    // Union is not valid
                    return false;
                }
            }
            else
            {
                if(!verifyStruct((std::unique_ptr<StructExprAST> const&)union_struct))
                {
                    // Struct is not valid
                    return false;
                }
            }
        } 
        
        if(!has_main)
        {
            code->addFunction(std::make_unique<FunctionAST>(
                std::make_unique<PrototypeAST>(std::make_unique<Viretoken>("main",tok_id), std::vector<std::unique_ptr<VariableDefAST>>()), 
                std::vector<std::unique_ptr<ExprAST>>()
                )
            );
            main_func_indx=funcs.size();
        }
        
        const auto& main_func=(std::unique_ptr<FunctionAST> const&)funcs[main_func_indx];
        for(unsigned int it=0; it<pre_stms.size(); ++it)
        {
            auto expr=std::move(pre_stms[it]);
            if(!verifyExpr((std::unique_ptr<ExprAST> const&)expr))
            {
                // Pre-execution statement is not valid
                return false;
            }

            main_func->insertStatement(std::move(expr));
        }

        return true;
    }

    bool VAnalyzer::verifyExpr(const std::unique_ptr<ExprAST>& expr)
    {
        switch(expr->asttype)
        {
            case ast_int: return verifyInt((const std::unique_ptr<IntExprAST>&)expr);
            case ast_float: return verifyFloat((const std::unique_ptr<FloatExprAST>&)expr);
            case ast_double: return verifyDouble((const std::unique_ptr<DoubleExprAST>&)expr);
            case ast_str: return verifyStr((const std::unique_ptr<StrExprAST>&)expr);
            case ast_char: return verifyChar((const std::unique_ptr<CharExprAST>&)expr);

            case ast_binop: return verifyBinop((const std::unique_ptr<BinaryExprAST>&)expr);
            case ast_unop: return verifyUnop((const std::unique_ptr<UnaryExprAST>&)expr);

            case ast_var: return verifyVar((const std::unique_ptr<VariableExprAST>&)expr);
            case ast_vardef: return verifyVarDef((const std::unique_ptr<VariableDefAST>&)expr);
            case ast_typedvar: return verifyTypedVar((const std::unique_ptr<TypedVarAST>&)expr);
            case ast_varassign: return verifyVarAssign((const std::unique_ptr<VariableAssignAST>&)expr);

            case ast_call: return verifyCall((const std::unique_ptr<CallExprAST>&)expr);

            case ast_for: return verifyFor((const std::unique_ptr<ForExprAST>&)expr);
            case ast_while: return verifyWhile((const std::unique_ptr<WhileExprAST>&)expr);
            case ast_array: return verifyArray((const std::unique_ptr<ArrayExprAST>&)expr);

            case ast_break: return verifyBreak((const std::unique_ptr<BreakExprAST>&)expr);
            case ast_continue: return verifyContinue((const std::unique_ptr<ContinueExprAST>&)expr);
            case ast_return: return verifyReturn((const std::unique_ptr<ReturnExprAST>&)expr);

            default: return false;
        }
    }
}