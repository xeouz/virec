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
        //scope.insert(std::make_pair(var->getName(), var));
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

        return current_func;
    }
    std::unique_ptr<types::Base> VAnalyzer::getFuncReturnType(const std::string& name)
    {
        types::Base const& type=(*getFunc(name)->getReturnType());
        return std::make_unique<types::Base>(type);
    }

    CodeAST* const VAnalyzer::getCode()
    {
        return codeast.get();
    }

    VariableDefAST* const VAnalyzer::getVar(const std::string& name)
    {
        return scope.at(name);
    }

    // !!- CHANGES REQUIRED -!!
    // str to be implemented
    std::unique_ptr<types::Base> VAnalyzer::getType(ExprAST* const& expr)
    {
        switch (expr->asttype)
        {
            case ast_int: return types::construct("int");
            case ast_float: return types::construct("float");
            case ast_double: return types::construct("double");
            case ast_char: return types::construct("char");

            case ast_binop: return getType(((BinaryExprAST*const&)expr)->getLHS());

            case ast_varincrdecr: 
            return std::make_unique<types::Base>(*getVar(((VariableExprAST*const&)expr)->getName())->getType());
            case ast_var: 
            return std::make_unique<types::Base>(*getVar(((VariableExprAST*const&)expr)->getName())->getType());

            case ast_call: return getFuncReturnType(((CallExprAST*const&)expr)->getName());

            case ast_array: return getType((ArrayExprAST*const&)expr);

            default: return types::construct("void");
        }
    }
    
    // !!- CHANGES REQUIRED -!!
    // unhandled dynamic array typing
    std::unique_ptr<types::Base> VAnalyzer::getType(ArrayExprAST* const& array)
    {
        const auto& vec = array->getElements();
        auto type=getType(vec[0].get());

        for(int i=0; i<vec.size(); ++i)
        {
            auto new_type=getType(vec[i].get());
            if(!types::isSame(type.get(), new_type.get()))
            {
                std::cout << "Error: Array element types do not match: " << *type << " " << *new_type << std::endl;
                return types::construct("void");
            }
            vec[i]->setType(std::move(new_type));
        }
        
        unsigned int len=((types::Array*)array->getType())->getLength();
        return std::make_unique<types::Array>(std::move(type), len);
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
                if(var->getValue()==nullptr && types::isSame(var->getType(), "auto"))
                {
                    // Requires a variable for definiton
                    unsigned char islet = var->isLet() ? 1 : 0;
                    builder->addError<errors::analyze_requires_type>
                    (this->code, islet, var->getName(), var->getLine(), var->getCharpos());
                    return false;
                }
            }
            
            auto* type=var->getType();
            std::unique_ptr<types::Base> value_type;

            auto const& value=var->getValue();
            bool is_auto=(type->getType()==types::TypeNames::Void);
            if(value==nullptr)
            {
                return true;
            }

            if(is_auto)
            {
                if(isvar)
                {
                    std::cout << "any type not implement yet" << std::endl;
                    type=getType(value).release();
                }
                else
                {
                    type=getType(var->getValue()).release();
                }
            }
            value_type=getType(value);

            if(!types::isSame(type, value_type.get()))    
            {
                std::cout << "Error: Type mismatch: " << *type << " " << *value_type << std::endl;
                return false;
            }

            var->setType(std::move(value_type));

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

        if(types::isSame(var->getType(), assign_type.get()))
        {
            // Type mismatch
            std::cout << "Type mismatch in assignment, types are: " << var->getType() << " and " << *assign_type << std::endl;
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

            auto argtype=getType(arg.get());
            if(!types::isSame(func_args[i]->getType(), argtype.get()))
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
        
        auto ret_expr_type=getType(ret->getValue());

        
        if(!types::isSame(ret_type.get(), ret_expr_type.get()))
        {
            std::cout << "Return type mismatch, expected " << *ret_type << " got " << *ret_expr_type << std::endl;
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

        if(types::isSame(proto->getReturnType(),"any") || types::isSame(proto->getReturnType(),"auto"))
        {
            // Type is not valid
            is_valid=false;
        }
        
        const auto& args=proto->getArgs();
        
        for(const auto& arg : args)
        {   
            if(types::isSame(arg->getType(),"auto") || types::isSame(arg->getType(),"any"))
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
        //std::cout << ("Verifying function " + func->getNameToken()->value) << std::endl;
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
            auto nametok=std::make_unique<Viretoken>("main", tok_func);
            auto rettok =std::make_unique<Viretoken>("int", tok_id);
            std::vector<std::unique_ptr<VariableDefAST>> args;
            code->addFunction(std::make_unique<FunctionAST>(
                std::make_unique<PrototypeAST>(std::move(nametok), std::move(args), std::move(rettok)), 
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
            case ast_int: return verifyInt((IntExprAST*const&)expr);
            case ast_float: return verifyFloat((FloatExprAST*const&)expr);
            case ast_double: return verifyDouble((DoubleExprAST*const&)expr);
            case ast_str: return verifyStr((StrExprAST*const&)expr);
            case ast_char: return verifyChar((CharExprAST*const&)expr);

            case ast_binop: return verifyBinop((BinaryExprAST*const&)expr);
            case ast_unop: return verifyUnop((UnaryExprAST*const&)expr);

            case ast_varincrdecr: return verifyIncrDecr((VariableIncrDecrAST*const&)expr);
            case ast_var: return verifyVar((VariableExprAST*const&)expr);
            case ast_vardef: return verifyVarDef((VariableDefAST*const&)expr);
            case ast_typedvar: return verifyTypedVar((TypedVarAST*const&)expr);
            case ast_varassign: return verifyVarAssign((VariableAssignAST*const&)expr);

            case ast_call: return verifyCall((CallExprAST*const&)expr);

            case ast_for: return verifyFor((ForExprAST*const&)expr);
            case ast_while: return verifyWhile((WhileExprAST*const&)expr);
            case ast_array: return verifyArray((ArrayExprAST*const&)expr);

            case ast_break: return verifyBreak((BreakExprAST*const&)expr);
            case ast_continue: return verifyContinue((ContinueExprAST*const&)expr);
            case ast_return: return verifyReturn((ReturnExprAST*const&)expr);

            case ast_ifelse: return verifyIf((IfExprAST*const&)expr);

            default: return false;
        }
    }
}