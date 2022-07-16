#include "analyzer.hpp"
#include <string>
#include <memory>
#include <vector>
#include <algorithm>

namespace vire
{
    bool VAnalyzer::isVarDefined(const std::string& name, bool check_globally_only)
    {
        for(const auto& expr :codeast->getPreExecutionStatements())
        {
            if(expr->asttype==ast_vardef)
            {
                if(((std::unique_ptr<VariableDefAST>const&)expr)->getName()==name)
                {
                    return true;
                }
            }
        }

        if(check_globally_only) return false;

        return scope.count(name)==true;
    }

    bool VAnalyzer::isFuncDefined(const std::string& name)
    {
        const auto& functions=codeast->getFunctions();
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
        const auto& classes=codeast->getClasses();
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

    void VAnalyzer::addVar(VariableDefAST* const& var)
    {
        scope.insert(std::make_pair(var->getName(), var));
    }
    void VAnalyzer::removeVar(VariableDefAST* const& var)
    {
        scope.erase(var->getName());
    }
    void VAnalyzer::addFunction(std::unique_ptr<FunctionBaseAST> func)
    {
        codeast->addFunction(std::move(func));
    }

    FunctionBaseAST* const VAnalyzer::getFunc(std::string const& name)
    {
        const auto& functions=codeast->getFunctions();
        for(int i=0; i<functions.size(); i++)
        {
            if(functions[i]->getName()==name)
            {
                return functions[i].get();
            }
        }

        return nullptr;
    }
    std::string const& VAnalyzer::getFuncReturnType(const std::string& name)
    {
        if(name=="")    return current_func->getType();
        else            return getFunc(name)->getType();
    }

    CodeAST* const VAnalyzer::getCode()
    {
        return codeast.get();
    }

    VariableDefAST* const VAnalyzer::getVar(const std::string& name)
    {
        return scope.at(name);
    }

    std::string VAnalyzer::getType(ExprAST* const& expr)
    {
        switch (expr->asttype)
        {
            case ast_int: return "int";
            case ast_float: return "float";
            case ast_double: return "double";
            case ast_str: return "str";
            case ast_char: return "char";

            case ast_binop: return getType(((BinaryExprAST*const&)expr)->getLHS());

            case ast_varincrdecr: return getVar(((VariableExprAST*const&)expr)->getName())->getType();
            case ast_var: return getVar(((VariableExprAST*const&)expr)->getName())->getType();

            default: return "";
        }
    }
    std::string VAnalyzer::getType(ArrayExprAST* const& array)
    {
        const auto& vec = array->getElements();
        std::string type=getType(vec[0].get());
        for(int i=0; i<vec.size(); ++i)
        {
            std::string newType=getType(vec[i].get());
            if(type!=newType)
            {
                return "any";
            }
        }

        return type;
    }

    // Helper functions
    ReturnExprAST* const getReturnStatement(std::vector<std::unique_ptr<ExprAST>> const& block)
    {
        for(auto const& expr : block)
        {
            if(expr->asttype==ast_return) 
            { return ((std::unique_ptr<ReturnExprAST> const&)expr).get(); }
        }

        return nullptr;
    }

    // Verification Functions
    bool VAnalyzer::verifyVar(VariableExprAST* const& var)
    {
        // Check if it is defined
        if(!isVarDefined(var->getName()))
        {
            // Var is not defined
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyIncrDecr(VariableIncrDecrAST* const& var)
    {
        // Check if it is defined
        if(!isVarDefined(var->getName()))
        {
            // Var is not defined
            return false;
        }
        
        return true;
    }
    bool VAnalyzer::verifyVarDef(VariableDefAST* const& var, bool check_globally_only)
    {
        if(!isVarDefined(var->getName(),check_globally_only))
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
            auto const& value=var->getValue();
            bool is_array=var->isArr() || (value!=nullptr && value->asttype==ast_array);
            bool is_auto=type=="auto" || type=="";

            if(value==nullptr)
            {
                return true;
            }

            if(is_auto)
            {
                if(isvar)
                {
                    std::cout << "any type not implement yet" << std::endl;
                    type=getType(value);
                }
                else
                {
                    type=getType(var->getValue());
                }
            }
            
            if(is_array)
            {
                auto arr=cast_static<ArrayExprAST>(var->moveValue());
                value_type=getType(arr.get());
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
            return true;
        }
        std::cout << "Variable already defined" << std::endl;
        // Variable is already defined
        return false;
    }
    bool VAnalyzer::verifyTypedVar(TypedVarAST* const& var)
    {
        return true;
    }
    bool VAnalyzer::verifyVarAssign(VariableAssignAST* const& assign)
    {
        if(!isVarDefined(assign->getName()))
        {
            // Var is not defined
            return false;
        }

        auto const& var=getVar(assign->getName());
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

    bool VAnalyzer::verifyInt(IntExprAST* const& int_) { return true; }
    bool VAnalyzer::verifyFloat(FloatExprAST* const& float_) { return true; }
    bool VAnalyzer::verifyDouble(DoubleExprAST* const& double_) { return true; }
    bool VAnalyzer::verifyChar(CharExprAST* const& char_) { return true; }
    bool VAnalyzer::verifyStr(StrExprAST* const& str) { return true; }
    bool VAnalyzer::verifyArray(ArrayExprAST* const& array)
    {
        const auto& elems=array->getElements();

        for(const auto& elem : elems)
        {
            if(!verifyExpr(elem.get()))
            {
                // Element is not valid
                return false;
            }
        }
        return true;
    }

    bool VAnalyzer::verifyFor(ForExprAST* const& for_)
    { 
        bool is_valid=true;
        const auto& init=for_->getInit();
        const auto& cond=for_->getCond();
        const auto& incr=for_->getIncr();
        
        if(!(init->asttype==ast_var || init->asttype==ast_varassign || init->asttype==ast_vardef))
        {
            // Init is not a variable definition
            is_valid=false;
        }
        if(!(cond->asttype==ast_var || cond->asttype==ast_unop || cond->asttype==ast_binop))
        {
            // Cond is not a boolean expression
            is_valid=false;
        }
        if(!(incr->asttype==ast_var || incr->asttype==ast_varassign || incr->asttype==ast_varincrdecr))
        {
            // Incr is not a step operation
            is_valid=false;
        }   
        
        if(!verifyExpr(init))
        {
            // Init is not valid
            is_valid=false;
        }
        if(!verifyExpr(cond))
        {
            // Cond is not valid
            is_valid=false;
        }
        if(!verifyExpr(incr))
        {
            // Incr is not valid
            is_valid=false;
        }

        if(!verifyBlock(for_->getBody()))
        {
            // Block is not valid
            return false;
        }
        
        return is_valid;
    }
    bool VAnalyzer::verifyWhile(WhileExprAST* const& while_)
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
    bool VAnalyzer::verifyBreak(BreakExprAST* const& break_) { return true; }
    bool VAnalyzer::verifyContinue(ContinueExprAST* const& continue_) { return true; }    

    bool VAnalyzer::verifyCall(CallExprAST* const& call)
    {
        bool is_valid=true;
        auto name=call->getName();

        if(isStdFunc(name))
        {
            is_valid=true;
        }
        else if(!isFuncDefined(name))
        {
            // Function is not defined
            is_valid=false;
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

            if(!verifyExpr(arg.get()))
            {
                // Argument is not valid
                is_valid=false;
            }

            if(func_args[i]->getType() != getType(arg.get()))
            {
                // Argument type mismatch
                is_valid=false;
            }
        }

        return is_valid;
    }

    bool VAnalyzer::verifyReturn(ReturnExprAST* const& ret)
    {
        const auto& ret_type=getFuncReturnType();
        if(ret_type!=getType(ret->getValue()))
        {
            std::cout << "Return type mismatch, expected " << ret_type << " got " << getType(ret->getValue()) << std::endl;
            // Type mismatch
            return false;
        }

        return true;
    }

    bool VAnalyzer::verifyBinop(BinaryExprAST* const& binop)
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
    bool VAnalyzer::verifyUnop(UnaryExprAST* const& unop)
    {
        const auto& expr=unop->getExpr();

        if(!verifyExpr(expr))
        {
            // Expr is not valid
            return false;
        }
        
        return true;
    }

    bool VAnalyzer::verifyPrototype(PrototypeAST* const& proto)
    {
        bool is_valid=true;

        if(isFuncDefined(proto->getName()))
        {
            // Function is already defined
            return false;
        }

        if(proto->getType()=="any" || proto->getType()=="auto")
        {
            // Type is not valid
            is_valid=false;
        }
        
        const auto& args=proto->getArgs();
        
        for(const auto& arg : args)
        {   
            if(arg->getType() == "auto" || arg->getType() == "any")
            {
                // Type is not valid
                is_valid=false;
            }

            if(!verifyVarDef(arg.get(),true))
            {
                std::cout << "Argument is not valid" << std::endl;
                // Argument is not valid
                is_valid=false;
            }
        }
        
        return is_valid;
    }
    bool VAnalyzer::verifyProto(PrototypeAST* const& proto)
    {
        if(!verifyPrototype(proto))
        {
            // Prototype is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyExtern(ExternAST* const& extern_)
    {
        if(!verifyPrototype(extern_->getProto()))
        {
            // Prototype is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyFunction(FunctionAST* const& func)
    {
        bool is_valid=true;

        if(!verifyPrototype(func->getProto()))
        {
            // Prototype is not valid
            is_valid=false;
        }

        if(!verifyBlock(func->getBody()))
        {
            // Block is not valid
            is_valid=false;
        }

        return is_valid;
    }

    bool VAnalyzer::verifyUnionStructBody(std::vector<std::unique_ptr<ExprAST>> const& body)
    {
        for(const auto& expr: body)
        {
            if(expr->asttype==ast_typedvar)
            {
                if(!verifyTypedVar(((std::unique_ptr<TypedVarAST> const&)expr).get()))
                {
                    // TypedVar is not valid
                    return false;
                }
            }
            else if(expr->asttype==ast_struct)
            {
                if(!verifyStruct(((std::unique_ptr<StructExprAST> const&)expr).get()))
                {
                    // Struct is not valid
                    return false;
                }
            }
            else if(expr->asttype==ast_union)
            {
                if(!verifyUnion(((std::unique_ptr<UnionExprAST> const&)expr).get()))
                {
                    // Union is not valid
                    return false;
                }
            }
        }

        return true;
    }
    bool VAnalyzer::verifyUnion(UnionExprAST* const& union_)
    {
        const auto& members=union_->getMembers();
        
        if(!verifyUnionStructBody(members))
        {
            // Union body is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyStruct(StructExprAST* const& struct_)
    {
        const auto& members=struct_->getMembers();
        
        if(!verifyUnionStructBody(members))
        {
            // Struct body is not valid
            return false;
        }

        return true;
    }

    bool VAnalyzer::verifyIfThen(IfThenExpr* const& if_then)
    {
        bool is_valid=true;
        const auto& cond=if_then->getCondition();
        const auto& then_block=if_then->getThenBlock();
        
        if(cond->asttype!=ast_var && cond->asttype!=ast_unop && cond->asttype!=ast_binop)
        {
            // Cond is not a boolean expression
            is_valid=false;
        }
        if(!verifyExpr(cond))
        {
            // Cond is not valid
            is_valid=false;
        }
        if(!verifyBlock(then_block))
        {
            // Then block is not valid
            is_valid=false;
        }

        return is_valid;
    }
    bool VAnalyzer::verifyIf(IfExprAST* const& if_)
    {
        bool is_valid=true;

        if(!verifyIfThen(if_->getIfThen()))
        {
            // IfThen is not valid
            is_valid=false;
        }
        for(const auto& else_if : if_->getElifLadder())
        {
            if(else_if->asttype!=ast_if)
            {
                if(!verifyIfThen(((std::unique_ptr<IfThenExpr> const&)else_if).get()))
                {
                    // ElseIf is not valid
                    is_valid=false;
                }
            }
            else
            {
                if(!verifyBlock(else_if->getThenBlock()))
                {
                    // ElseIf block is not valid
                    is_valid=false;
                }
            }
        }
        
        return is_valid;
    }

    bool VAnalyzer::verifyUnsafe(UnsafeExprAST* const& unsafe)
    {
        const auto& block=unsafe->getBody();
        if(!verifyBlock(block))
        {
            // Block is not valid
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyReference(ReferenceExprAST* const& reference)
    {
        const auto& expr=reference->getVar();
        if(!verifyExpr(expr))
        {
            // Expr is not valid
            return false;
        }
        return true;
    }

    bool VAnalyzer::verifyClass(ClassAST* const& cls)
    {
        if(isClassDefined(cls->getName()))
        {
            // Class is already defined
            return false;
        }
        
        const auto& members=cls->getMembers();
        const auto& funcs=cls->getFunctions();

        bool is_valid=true;
        for(auto const& member : members)
        {
            if(!verifyVarDef((VariableDefAST*const&)member))
            {
                // Member is not valid
                return false;
            }
        }
        for(auto const& func : funcs)
        {
            if(func->is_extern())
            {
                is_valid=false;
            }
            else if(func->is_proto())
            {
                if(!verifyPrototype((PrototypeAST*const&)func))
                {
                    // Prototype is not valid
                    return false;
                }
            }
            else
            {
                const auto& func_cast=(FunctionAST*const&)func;

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
        std::vector<VariableDefAST*> refcnt;
        for(auto const& expr : block)
        {
            auto* ptr=expr.get();
            if(!verifyExpr(ptr))
            {
                // Expr is not valid
                return false;
            }

            if(expr->asttype==ast_vardef)
            {
                addVar((VariableDefAST*)ptr);
                refcnt.push_back((VariableDefAST*)ptr);
            }
        }
        for(auto const& var : refcnt)
        {
            removeVar(var);
        }
        
        return true;
    }

    bool VAnalyzer::verifyCode(std::unique_ptr<CodeAST> code)
    {
        bool is_valid=true;
        codeast=std::move(code);

        auto classes=codeast->moveClasses();
        auto funcs=codeast->moveFunctions();
        auto union_structs=codeast->moveUnionStructs();
        auto pre_stms=codeast->movePreExecutionStatements();

        bool has_main=false;
        unsigned int main_func_indx=0;
        for(const auto& cls : classes)
        {
            if(!verifyClass(cls.get()))
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
                if(!verifyExtern(((std::unique_ptr<ExternAST> const&)func).get()))
                {
                    // Extern is not valid
                    is_valid=false;
                }
            }
            else if(func->is_proto())
            {
                if(!verifyPrototype(((std::unique_ptr<PrototypeAST> const&)func).get()))
                {
                    // Prototype is not valid
                    is_valid=false;
                }
            }
            else
            {
                auto const& casted_func=((std::unique_ptr<FunctionAST>const&)func).get();
                current_func=casted_func;
                if(!verifyFunction(casted_func))
                {
                    // Function is not valid
                    is_valid=false;
                }

                if(casted_func->getName()=="main")
                {
                    has_main=true;
                    main_func_indx=it;
                }
            }

            addFunction(std::move(funcs[it]));
        }
        for(const auto& union_struct : union_structs)
        {
            if(union_struct->asttype==ast_union)
            {
                if(!verifyUnion(((std::unique_ptr<UnionExprAST> const&)union_struct).get()))
                {
                    // Union is not valid
                    is_valid=false;
                }
            }
            else
            {
                if(!verifyStruct(((std::unique_ptr<StructExprAST> const&)union_struct).get()))
                {
                    // Struct is not valid
                    is_valid=false;
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
            if(!verifyExpr(((std::unique_ptr<ExprAST> const&)expr).get()))
            {
                // Pre-execution statement is not valid
                is_valid=false;
            }

            main_func->insertStatement(std::move(expr));
        }

        return is_valid;
    }

    bool VAnalyzer::verifyExpr(ExprAST* const& expr)
    {
        switch(expr->asttype)
        {
            case ast_int: return verifyInt(((std::unique_ptr<IntExprAST> const&)expr).get());
            case ast_float: return verifyFloat(((std::unique_ptr<FloatExprAST> const&)expr).get());
            case ast_double: return verifyDouble(((std::unique_ptr<DoubleExprAST> const&)expr).get());
            case ast_str: return verifyStr(((std::unique_ptr<StrExprAST> const&)expr).get());
            case ast_char: return verifyChar(((std::unique_ptr<CharExprAST> const&)expr).get());

            case ast_binop: return verifyBinop(((std::unique_ptr<BinaryExprAST> const&)expr).get());
            case ast_unop: return verifyUnop(((std::unique_ptr<UnaryExprAST> const&)expr).get());

            case ast_varincrdecr: return verifyIncrDecr(((std::unique_ptr<VariableIncrDecrAST> const&)expr).get());
            case ast_var: return verifyVar(((std::unique_ptr<VariableExprAST> const&)expr).get());
            case ast_vardef: return verifyVarDef(((std::unique_ptr<VariableDefAST> const&)expr).get());
            case ast_typedvar: return verifyTypedVar(((std::unique_ptr<TypedVarAST> const&)expr).get());
            case ast_varassign: return verifyVarAssign(((std::unique_ptr<VariableAssignAST>&)expr).get());

            case ast_call: return verifyCall(((std::unique_ptr<CallExprAST> const&)expr).get());

            case ast_for: return verifyFor(((std::unique_ptr<ForExprAST> const&)expr).get());
            case ast_while: return verifyWhile(((std::unique_ptr<WhileExprAST> const&)expr).get());
            case ast_array: return verifyArray(((std::unique_ptr<ArrayExprAST> const&)expr).get());

            case ast_break: return verifyBreak(((std::unique_ptr<BreakExprAST> const&)expr).get());
            case ast_continue: return verifyContinue(((std::unique_ptr<ContinueExprAST> const&)expr).get());
            case ast_return: return verifyReturn(((std::unique_ptr<ReturnExprAST> const&)expr).get());

            case ast_ifelse: return verifyIf(((std::unique_ptr<IfExprAST> const&)expr).get());

            default: return false;
        }
    }
}