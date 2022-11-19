#include "analyzer.hpp"

#include <string>
#include <memory>
#include <vector>
#include <algorithm>

namespace vire
{
    bool VAnalyzer::isVariableDefined(const proto::IName& name)
    {
        return scope.count(name.get());
    }

    bool VAnalyzer::isStructDefined(const std::string& name)
    {
        return types::isTypeinMap(name);
    }
    bool VAnalyzer::isFuncDefined(const std::string& name)
    {
        const auto& functions=ast->getFunctions();
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
        const auto& classes=ast->getClasses();
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

    void VAnalyzer::defineVariable(VariableDefAST* const var)
    {
        scope.insert(std::make_pair(var->getName(), var));
        
        if(scope_varref != nullptr)
        {
            scope_varref->push_back(var);
        }
    }
    void VAnalyzer::undefineVariable(VariableDefAST* const var)
    {
        undefineVariable(var->getName());
    }
    void VAnalyzer::undefineVariable(proto::IName const& name)
    {
        scope.erase(name.get());
    }

    void VAnalyzer::addFunction(std::unique_ptr<FunctionBaseAST> func)
    {
        ast->addFunction(std::move(func));
    }

    ModuleAST* const VAnalyzer::getSourceModule()
    {
        return ast.get();
    }

    FunctionBaseAST* const VAnalyzer::getFunction(const std::string& name)
    {
        return getFunction(proto::IName(name, ""));
    }
    VariableDefAST* const VAnalyzer::getVariable(const std::string& name)
    {
        return getVariable(proto::IName(name, ""));
    }
    StructExprAST* const VAnalyzer::getStruct(const std::string& name)
    {
        return getStruct(proto::IName(name, ""));
    }
    FunctionBaseAST* const VAnalyzer::getFunction(const proto::IName& name)
    {
        const auto& functions=ast->getFunctions();

        for(int i=0; i<functions.size(); i++)
        {
            if(functions[i]->getIName()==name)
            {
                return functions[i].get();
            }
        }

        if(current_func)
        {
            if(current_func->getIName()==name || name.name=="")
            {
                return current_func;
            }
        }
        
        std::cout << "Function `" << name << "` not found" << std::endl;
        return nullptr;
    }
    VariableDefAST* const VAnalyzer::getVariable(const proto::IName& name)
    {
        if(!isVariableDefined(name))
        {
           std::cout << "Variable `" << name << "` not found" << std::endl;
           return nullptr;
        }
        return scope.at(name.get());
    }
    StructExprAST* const VAnalyzer::getStruct(const proto::IName& name)
    {
        if(!isStructDefined(name.get()))
        {
            std::cout << "Struct with name not defined: " << name << std::endl;
            return nullptr;
        }

        auto const& structs=ast->getUnionStructs();
        for(auto const& expr : structs)
        {
            if(expr->asttype==ast_struct)
            {
                auto* st=(StructExprAST*)expr.get();

                if(st->getIName().name==name)
                {
                    return st;
                }
            }
        }

        std::cout << "Could not find struct with name: " << name << std::endl;

        return nullptr;
    }
    
    // !!- CHANGES REQUIRED -!!
    // str to be implemented
    types::Base* VAnalyzer::getType(ExprAST* const expr)
    {
        switch (expr->asttype)
        {
            case ast_int: return expr->getType();
            case ast_float: return expr->getType();
            case ast_double: return expr->getType();
            case ast_char: return expr->getType();
            case ast_bool: return expr->getType();

            case ast_binop:
            {
                auto* binop=((BinaryExprAST*)expr);
                auto* t=binop->getOpType();

                if(t==nullptr)
                {
                    auto* lhs=getType(binop->getLHS());
                    auto* rhs=getType(binop->getRHS());

                    if(binop->getOp()->type==tok_div)
                    {
                        bool lhs_double=lhs->getType()==types::TypeNames::Double;
                        bool rhs_double=rhs->getType()==types::TypeNames::Double;

                        if(lhs_double || rhs_double)
                        {
                            binop->setType(types::construct(types::TypeNames::Double));
                            return binop->getType();
                        }
                        else
                        {
                            binop->setType(types::construct(types::TypeNames::Float));
                            return binop->getType();
                        }
                    }
                    else
                    {
                        return (lhs->precedence>rhs->precedence)?lhs:rhs;
                    }
                }

                return t;
            }

            case ast_incrdecr: 
            {
                auto* incrdecr=(IncrementDecrementAST*)expr;
                return getType(incrdecr->getExpr());
            }
            case ast_var:
            {
                auto* var=getVariable(((VariableExprAST*)expr)->getIName());
                return var->getType();
            }
            case ast_array_access:
            {
                auto* expr_cast=(VariableArrayAccessAST*)expr;
                auto* array_type=(types::Array*)getType(expr_cast->getExpr());

                // Loop over the indices and get the type of each index
                for(int i=0; i<expr_cast->getIndices().size(); ++i)
                {
                    array_type=(types::Array*)array_type->getChild();
                }

                return array_type;
            }

            case ast_call: return getFunction(((CallExprAST*)expr)->getName())->getReturnType();

            case ast_array: return getType((ArrayExprAST*)expr);

            case ast_type_access:
            {
                auto* access=(TypeAccessAST*)expr;
                
                auto* st_type=getType(access->getParent());
                auto name=((types::Void*)st_type)->getName();
                auto* st=getStruct(name);

                while(access->getChild()->asttype==ast_type_access)
                {
                    name=access->getName();
                    access=(TypeAccessAST*)access->getChild();
                    st=(StructExprAST*)st->getMember(name);
                }

                return st->getMember(access->getChild()->getIName())->getType();
            }

            default:
            {
                std::cout<<"Error: Unknown type in getType()"<<std::endl;
                return nullptr;
            }
        }
    }
    
    // !!- CHANGES REQUIRED -!!
    // unhandled dynamic array typing
    types::Base* VAnalyzer::getType(ArrayExprAST* const array)
    {
        const auto& vec=array->getElements();
        auto type=types::copyType(getType(vec[0].get()));

        for(int i=0; i<vec.size(); ++i)
        {
            auto* new_type=getType(vec[i].get());

            if(!types::isSame(type.get(), new_type))
            {
                std::cout << "Error: Array element types do not match: " << *type << " " << *new_type << std::endl;
                return nullptr;
            }
        }
        
        unsigned int len=((types::Array*)array->getType())->getLength();

        auto type_uptr=std::make_unique<types::Array>(std::move(type), len);
        array->setType(std::move(type_uptr));

        return array->getType();
    }

    // Helper functions
    ReturnExprAST* const VAnalyzer::getReturnStatement(std::vector<std::unique_ptr<ExprAST>> const& block)
    {
        for(auto const& expr : block)
        {
            if(expr->asttype==ast_return) 
            { return ((std::unique_ptr<ReturnExprAST> const&)expr).get(); }
        }

        return nullptr;
    }
    std::unique_ptr<ExprAST> VAnalyzer::tryCreateImplicitCast(types::Base* target, types::Base* base, std::unique_ptr<ExprAST> expr)
    {
        bool types_are_user_defined=(types::isUserDefined(target) || types::isUserDefined(base));
        bool types_are_arrays=(target->getType()==types::TypeNames::Array || base->getType()==types::TypeNames::Array);
        if(!types_are_user_defined && !types_are_arrays)
        {
            auto src_type=types::copyType(base);
            auto dst_type=types::copyType(target);

            auto new_cast_value=std::make_unique<CastExprAST>(std::move(expr), std::move(dst_type), true);
            new_cast_value->setSourceType(std::move(src_type));

            base=new_cast_value->getSourceType();
            target=new_cast_value->getDestType();
            
            if(base->getSize() > target->getSize())
            {
                std::cout << "Warning: Analysis: Truncation, possible data loss while converting from "
                << *base << " to " << *target << std::endl;
            }
            else if(types::isTypeFloatingPoint(base) && !types::isTypeFloatingPoint(target))
            {
                std::cout << "Warning: Analysis: Decimal (Floating point) to Integer, possible data loss while converting from "
                << *base << " to " << *target << std::endl;
            }

            return std::move(new_cast_value);
        }
        else
        {
            return nullptr;
        }
    }

    // Verification Functions
    bool VAnalyzer::verifyVar(VariableExprAST* const var)
    {
        // Check if it is defined
        if(!isVariableDefined(var->getIName()))
        {
            // Variable is not defined
            std::cout << "Variable " << var->getName() << " defined" << std::endl;
            return false;
        }
        
        return true;
    }
    bool VAnalyzer::verifyIncrementDecrement(IncrementDecrementAST* const incrdecr)
    {
        // Check if it is defined
        auto const& expr=incrdecr->getExpr();

        if(!verifyExpr(expr))
        {
            return false;
        }

        if(!types::isNumericType(getType(expr)))
        {
            return false;
        }
        
        return true;
    }
    bool VAnalyzer::verifyVariableDef(VariableDefAST* const var, bool add_to_scope)
    {
        if(!isVariableDefined(var->getName()))
        {
            bool is_var=!(var->isLet() || var->isConst());

            if(!is_var)
            {
                bool type_is_void=false;
                bool type_is_given=true;
                auto* ty=var->getType();

                type_is_void=(ty->getType()==types::TypeNames::Void);
                if(type_is_void)
                {
                    auto* voidty=(types::Void*)ty;
                    if(types::isTypeinMap(voidty->getName()))
                    {
                        type_is_given=true;
                    }
                    else
                    {
                        type_is_given=false;
                    }
                }

                if(var->getValue()==nullptr && !type_is_given)
                {
                    // Requires a variable for definiton
                    unsigned char islet = var->isLet() ? 1 : 0;
                    builder->addError<errors::analyze_requires_type>
                    (this->code, islet, var->getName(), var->getLine(), var->getCharpos());

                    return false;
                }
            }
            types::Base* type=var->getType();
            types::Base* value_type;

            auto const& value=var->getValue();

            if(value==nullptr)
            {
                var->refreshType();

                if(add_to_scope)
                    defineVariable(var);

                return true;
            }

            bool is_auto=(type->getType()==types::TypeNames::Void);
            if(is_auto && is_var)
            {
                std::cout << "`any` type not implement yet" << std::endl;
            }
            
            if(!verifyExpr(value))
            {
                return false;
            }
            
            value_type=getType(value);
            if(value_type->getType()==types::TypeNames::Array && value->asttype!=ast_array)
            {
                std::cout << "Error: Cannot assign arrays to variables without array literal" << std::endl;
                return false;
            }
            
            if(!is_auto)
            {
                if(!types::isSame(type, value_type))    
                {
                    auto new_cast_value=tryCreateImplicitCast(type, value_type, var->moveValue());
                        
                    if(!new_cast_value)
                    {
                        std::cout << "Error: VarDef Type Mismatch: " << *type << " and " << *value_type << std::endl;
                        return false;
                    }
                    else
                    {
                        var->setValue(std::move(new_cast_value));
                    }
                }
            }
            else
            {
                auto value_type_uptr=types::copyType(value_type);

                var->getValue()->setType(std::move(value_type_uptr));
                var->setUseValueType(true);
            }
            
            if(add_to_scope)
                defineVariable(var);
            
            return true;
        }
        
        std::cout << "Variable " << var->getName() << " already defined" << std::endl;
        // Variable is already defined
        return false;
    }
    bool VAnalyzer::verifyVarAssign(VariableAssignAST* const assign)
    {
        bool is_valid=true;
        
        if(!verifyExpr(assign->getLHS()))
        {
            // Var is not defined
            return false;
        }
        if(!verifyExpr(assign->getRHS()))
        {
            return false;
        }

        auto* lhs_type=getType(assign->getLHS());
        auto* rhs_type=getType(assign->getRHS());

        if(!types::isSame(lhs_type, rhs_type))
        {
            auto cast=tryCreateImplicitCast(lhs_type, rhs_type, assign->moveRHS());

            if(!cast)
            {
                std::cout << "Error: Assigment: Variable and Value types do not match" << std::endl;
                return false;
            }
            else
            {
                assign->setRHS(std::move(cast));
            }
        }

        assign->getLHS()->setType(types::copyType(lhs_type));
        assign->getRHS()->setType(types::copyType(rhs_type));
        
        return is_valid;
    }
    bool VAnalyzer::verifyVarArrayAccess(VariableArrayAccessAST* const access)
    {
        if(!verifyExpr(access->getExpr()))
        {
            // Expr is not valid
            return false;
        }
        
        auto const& indices=access->getIndices();
        auto* type=getType(access->getExpr());
        
        if(type->getType()!=types::TypeNames::Array)
        {
            std::cout << "Error: Variable is not an array" << std::endl;
            return false;
        }
        else
        {
            auto* array_type=(types::Array*)type;
            if(indices.size()!=array_type->getDepth())
            {
                std::cout << "Error: Array index mismatch" << std::endl;
                return false;
            }
            else
            {
                auto* child_array_type=array_type;
                for(auto const& index : indices)
                {
                    auto* index_type=getType(index.get());
                    if(index_type->getType() == types::TypeNames::Int)
                    {
                        auto* index_cast=(IntExprAST*)index.get();
                        if(index_cast->getValue() >= child_array_type->getLength())
                        {
                            std::cout << "Error: Array index out of bounds" << std::endl;
                            return false;
                        }
                    }
                    else
                    {
                        std::cout << "Error: Array index is not an integer, and not " << *index_type << std::endl;
                        return false;
                    }
                    index->setType(types::copyType(index_type));

                    child_array_type=(types::Array*)child_array_type->getChild();
                }
            }
        }
        
        access->setType(types::copyType(type));

        return true;
    }

    bool VAnalyzer::verifyInt(IntExprAST* const int_) { return true; }
    bool VAnalyzer::verifyFloat(FloatExprAST* const float_) { return true; }
    bool VAnalyzer::verifyDouble(DoubleExprAST* const double_) { return true; }
    bool VAnalyzer::verifyChar(CharExprAST* const char_) { return true; }
    bool VAnalyzer::verifyStr(StrExprAST* const str) { return true; }
    bool VAnalyzer::verifyBool(BoolExprAST* const bool_) { return true; }
    bool VAnalyzer::verifyArray(ArrayExprAST* const array)
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

    bool VAnalyzer::verifyFor(ForExprAST* const for_)
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
        if(!(incr->asttype==ast_var || incr->asttype==ast_varassign || incr->asttype==ast_incrdecr))
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
    bool VAnalyzer::verifyWhile(WhileExprAST* const while_)
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
    bool VAnalyzer::verifyBreak(BreakExprAST* const break_) { return true; }
    bool VAnalyzer::verifyContinue(ContinueExprAST* const continue_) { return true; }    

    bool VAnalyzer::verifyCall(CallExprAST* const call)
    {
        bool is_valid=true;
        auto name=call->getName();

        bool is_recursive_call=false;
        if(current_func)
        {
            if(current_func->getName()==name)
            {
                is_recursive_call=true;
            }
        }

        if(!isFuncDefined(name) && !is_recursive_call)
        {
            std::cout << "Function `" << name << "` is not defined" << std::endl;
            // Function is not defined
            return false;
        }

        if(call->getIName().name == "main")
        {
            std::cout << "Verification Error: Cannot call the main function, it is an entry point" << std::endl;
            is_valid=false;
        }

        auto args=call->moveArgs();
        const auto& func_args=getFunction(name)->getArgs();

        if(args.size() != func_args.size())
        {
            // Argument count mismatch
            is_valid=false;
        }

        for(unsigned int i=0; i<args.size(); ++i)
        {
            auto arg=std::move(args[i]);

            if(!verifyExpr(arg.get()))
            {
                // Argument is not valid
                is_valid=false;
                continue;
            }

            // This is to be changed, implemented for arguments with default value
            auto* arg_type=getType(arg.get());
            if(!types::isSame(func_args[i]->getType(), arg_type))
            {
                auto cast=tryCreateImplicitCast(func_args[i]->getType(), arg_type, std::move(arg));

                if(!cast)
                {
                    std::cout << "Error: Function call type mismatch, " << *func_args[i]->getType() << " : " << *arg_type << std::endl;
                    is_valid=false;
                }
                else
                {
                    arg=std::move(cast);
                }
            }

            args[i]=std::move(arg);
        }
        
        if(is_valid)
        {
            call->setArgs(std::move(args));
        }

        return is_valid;
    }

    bool VAnalyzer::verifyReturn(ReturnExprAST* const ret)
    {
        auto const& ret_type=getFunction(ret->getName())->getReturnType();

        if(!verifyExpr(ret->getValue()))
        {
            // Return value is not valid
            return false;
        }

        auto* ret_expr_type=getType(ret->getValue());
        
        if(!types::isSame(ret_type, ret_expr_type))
        {
            auto cast=tryCreateImplicitCast(ret_expr_type, ret_type, ret->moveValue());

            if(!cast)
            {
                std::cout << "Error: Return type mismatch, " << *ret_expr_type << " : " << *ret_type << std::endl;
                return false;
            }
            else
            {
                ret->setValue(std::move(cast));
            }
        }

        return true;
    }

    bool VAnalyzer::verifyBinop(BinaryExprAST* const binop)
    {
        const auto& left=binop->getLHS();
        const auto& right=binop->getRHS();

        bool is_valid=true;

        if(!verifyExpr(left))
        {
            // Left is not valid
            is_valid=false;
        }
        if(!verifyExpr(right))
        {
            // Right is not valid
            is_valid=false;
        }

        if(is_valid)
        {
            auto* left_type=getType(left);
            auto* right_type=getType(right);

            left->setType(types::copyType(left_type));
            right->setType(types::copyType(right_type));

            left_type=left->getType();
            right_type=right->getType();
            
            if(!types::isSame(left_type, right_type))
            {
                bool left_is_fp=types::isTypeFloatingPoint(left_type);
                bool right_is_fp=types::isTypeFloatingPoint(right_type);

                if(left_is_fp xor right_is_fp)
                {
                    if(left_is_fp)
                    {
                        binop->setRHS(tryCreateImplicitCast(left_type, right_type, binop->moveRHS()));
                    }
                    else
                    {
                        binop->setLHS(tryCreateImplicitCast(right_type, left_type, binop->moveLHS()));
                    }
                }
                else if(left_type->getSize() > right_type->getSize())
                {
                    binop->setRHS(tryCreateImplicitCast(left_type, right_type, binop->moveRHS()));
                }
                else
                {
                    binop->setLHS(tryCreateImplicitCast(right_type, left_type, binop->moveLHS()));
                }
            }

            binop->setType(types::copyType(binop->getLHS()->getType()));
        }

        return is_valid;
    }
    bool VAnalyzer::verifyUnop(UnaryExprAST* const unop)
    {
        const auto& expr=unop->getExpr();

        if(!verifyExpr(expr))
        {
            // Expr is not valid
            return false;
        }
        
        return true;
    }

    bool VAnalyzer::verifyPrototype(PrototypeAST* const proto)
    {
        bool is_valid=true;

        if(isFuncDefined(proto->getName()))
        {
            // Function is already defined
            return false;
        }

        if(types::isSame(proto->getReturnType(), "any") /*|| types::isSame(proto->getReturnType(), "auto")*/)
        {
            std::cout << "Function type cannot be `" << *proto->getReturnType() << "`" << std::endl;
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

            if(!verifyVariableDef(arg.get(), false))
            {
                std::cout << "Argument is not valid" << std::endl;

                // Argument is not valid
                is_valid=false;
            }
        }

        return is_valid;
    }
    bool VAnalyzer::verifyProto(PrototypeAST* const proto)
    {
        if(!verifyPrototype(proto))
        {
            // Prototype is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyExtern(ExternAST* const extern_)
    {
        if(!verifyPrototype(extern_->getProto()))
        {
            // Prototype is not valid
            return false;
        }
        
        return true;
    }
    bool VAnalyzer::verifyFunction(FunctionAST* const func)
    {
        //std::cout << ("Verifying function " + func->getNameToken()->value) << std::endl;
        bool is_valid=true;

        if(!verifyPrototype(func->getProto()))
        {
            // Prototype is not valid
            is_valid=false;
        }

        for(auto const& var: func->getArgs())
        {
            defineVariable(var.get());
        }

        if(!verifyBlock(func->getBody()))
        {
            // Block is not valid
            is_valid=false;
        }
        
        for(auto const& var: func->getArgs())
        {
            undefineVariable(var.get());
        }

        return is_valid;
    }

    bool VAnalyzer::verifyUnionStructBody(std::vector<ExprAST*> const& body)
    {
        bool is_valid=true;

        std::map<std::string, VariableDefAST*> scope;

        for(auto* expr: body)
        {
            if(expr->asttype==ast_vardef)
            {
                auto* var=(VariableDefAST*)expr;
                if(scope.count(var->getName())>0)
                {
                    std::cout << "Redeclaration of variable in struct" << std::endl;
                    is_valid=false;
                }
                else
                {
                    scope.insert(std::make_pair(var->getName(),var));
                }
            }
            else if(expr->asttype==ast_struct)
            {
                auto* struct_=(StructExprAST*)expr;
                struct_->setName("_"+struct_->getName());

                if(scope.count(struct_->getName())>0)
                {
                    std::cout << "Redeclaration of struct-variable in struct" << std::endl;
                    is_valid=false;
                }
                if(!verifyUnionStructBody(struct_->getMembersValues()))
                {
                    // Struct is not valid
                    is_valid=false;
                }

                unsigned int size=0;
                for(auto const& member : struct_->getMembersValues())
                {
                    size+=member->getType()->getSize();
                }

                struct_->getType()->setSize(size);
            }
            else if(expr->asttype==ast_union)
            {
                auto* union_=(UnionExprAST*)expr;
                union_->setName("_"+union_->getName());

                if(scope.count(union_->getName())>0)
                {
                    std::cout << "Redeclaration of union-variable in struct" << std::endl;
                    is_valid=false;
                }
                if(!verifyUnionStructBody(union_->getMembersValues()))
                {
                    // Union is not valid
                    is_valid=false;
                }
            }
        }

        return is_valid;
    }
    bool VAnalyzer::verifyUnion(UnionExprAST* const union_)
    {
        auto const& members=union_->getMembersValues();
        
        if(!verifyUnionStructBody(members))
        {
            // Union body is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyStruct(StructExprAST* const struct_)
    {
        bool is_valid=true;

        auto const& members=struct_->getMembersValues();
        if(!verifyUnionStructBody(members))
        {
            // Struct body is not valid
            return is_valid=false;
        }

        unsigned int size=0;
        for(auto const& member : members)
        {
            size+=member->getType()->getSize();
        }

        if(!is_valid)   return is_valid;

        std::string st_name=struct_->getName();
        if(!types::isTypeinMap(st_name))
        {
            types::addTypeToMap(st_name);
            types::addTypeSizeToMap(st_name, size);
        }
        else
        {
            std::cout << "Struct already defined" << std::endl;
            is_valid=false;
        }

        return is_valid;
    }
    bool VAnalyzer::verifyTypeAccess(TypeAccessAST* const access)
    {
        bool is_valid=true;

        // Load the struct
        StructExprAST* st=nullptr;

        auto* ptype=getType(access->getParent());
        if(ptype->getType() != types::TypeNames::Custom)
        {
            std::cout << "Parent is not a type" << std::endl;
            return false;
        }
        
        auto* ptype_custom=(types::Custom*)ptype;
        if(!types::isTypeinMap(ptype_custom->getName()))
        {
            std::cout << "Type " << *ptype_custom << " is not defined" << std::endl;
            return false;
        }

        access->getParent()->setType(types::copyType(ptype_custom));
        st=getStruct(ptype_custom->getName());

        IdentifierExprAST* possible_access=access;
        ExprAST* possible_struct_child=st;

        while(possible_access->asttype==ast_type_access)
        {
            auto const* casted_pos_access=(TypeAccessAST*)possible_access;
            auto* child=(TypeAccessAST*)possible_access;

            if(possible_struct_child->asttype!=ast_struct)
            {
                std::cout << "The type is not a struct" << std::endl;
                is_valid=false;
                break;
            }

            auto* casted_pos_stchild=(StructExprAST*)possible_struct_child;

            if(!casted_pos_stchild->isMember(child->getIName()))
            {
                std::cout << "No member as `" << child->getIName().name << "` in struct `" << casted_pos_stchild->getName() << "`." << std::endl;
                is_valid=false;
                break;
            }
            else
            {
                possible_struct_child->setType(std::make_unique<types::Custom>(casted_pos_stchild->getName(), 1));
                casted_pos_access->getParent()->setType(std::make_unique<types::Custom>(casted_pos_stchild->getName(), 1));
                possible_access=child->getChild();
                possible_struct_child=casted_pos_stchild->getMember(child->getIName());
            }
        }

        // Set the type for the tail of the access
        possible_access->setType(types::copyType(possible_struct_child->getType()));

        if(is_valid)
        {
            auto* type=getType(access);
            access->setType(types::copyType(type));
        }

        return is_valid;
    }

    bool VAnalyzer::verifyIfThen(IfThenExpr* const if_then)
    {
        bool is_valid=true;
        const auto& cond=if_then->getCondition();
        const auto& then_block=if_then->getThenBlock();
        
        auto* cond_type=getType(cond);

        if(cond_type->getType()!=types::TypeNames::Bool)
        {
            auto bool_type=types::construct(types::TypeNames::Bool);
            auto cast=tryCreateImplicitCast(bool_type.get(), cond_type, if_then->moveCondition());

            if(!cast)
            {
                std::cout << "Condition needs to be of a boolean type or a numeric type";
                is_valid=false;
            }
            else
            {
                if_then->setCondition(std::move(cast));
            }
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
    bool VAnalyzer::verifyIf(IfExprAST* const if_)
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

    bool VAnalyzer::verifyUnsafe(UnsafeExprAST* const unsafe)
    {
        const auto& block=unsafe->getBody();
        if(!verifyBlock(block))
        {
            // Block is not valid
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyReference(ReferenceExprAST* const reference)
    {
        const auto& expr=reference->getVariable();
        if(!verifyExpr(expr))
        {
            // Expr is not valid
            return false;
        }
        return true;
    }

    bool VAnalyzer::verifyClass(ClassAST* const cls)
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
            if(!verifyVariableDef((VariableDefAST*const&)member))
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
        auto* refscope=new std::vector<VariableDefAST*>();
        this->scope_varref=refscope;

        for(auto const& expr : block)
        {
            auto* ptr=expr.get();
            if(!verifyExpr(ptr))
            {
                // Expr is not valid
                return false;
            }
        }
        for(const auto& var : *refscope)
        {
            undefineVariable(var);
        }

        this->scope_varref=nullptr;
        delete refscope;
        
        return true;
    }

    bool VAnalyzer::verifySourceModule(std::unique_ptr<ModuleAST> code)
    {
        if(!code)
        {
            return false;
        }

        bool is_valid=true;
        ast=std::move(code);

        auto classes=ast->moveClasses();
        auto funcs=ast->moveFunctions();
        auto union_structs=ast->moveUnionStructs();
        auto pre_stms=ast->movePreExecutionStatements();

        bool has_main=false;
        unsigned int main_func_indx=0;

        // Verify all unions and structs
        for(unsigned int it=0; it<union_structs.size(); ++it)
        {
            const auto& union_struct=union_structs[it];
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

            ast->addUnionStruct(std::move(union_structs[it]));
        }

        // Verify all functions
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

        // Verify all statements in global scope
        auto* global_refscope=new std::vector<VariableDefAST*>();
        this->scope_varref=global_refscope;
        for(const auto& expr : pre_stms)
        {
            if(!verifyExpr(expr.get()))
            {
                is_valid=false;
            }
        }
        this->scope_varref=nullptr;
        delete global_refscope;

        ast->addPreExecutionStatements(std::move(pre_stms));

        return is_valid;
    }

    bool VAnalyzer::verifyExpr(ExprAST* const expr)
    {
        switch(expr->asttype)
        {
            case ast_int: return verifyInt((IntExprAST*const&)expr);
            case ast_float: return verifyFloat((FloatExprAST*const&)expr);
            case ast_double: return verifyDouble((DoubleExprAST*const&)expr);
            case ast_str: return verifyStr((StrExprAST*const&)expr);
            case ast_bool: return verifyBool((BoolExprAST*const&)expr);
            case ast_char: return verifyChar((CharExprAST*const&)expr);

            case ast_binop: return verifyBinop((BinaryExprAST*const&)expr);
            case ast_unop: return verifyUnop((UnaryExprAST*const&)expr);

            case ast_incrdecr: return verifyIncrementDecrement((IncrementDecrementAST*const&)expr);
            case ast_var: return verifyVar((VariableExprAST*const&)expr);
            case ast_vardef: return verifyVariableDef((VariableDefAST*const&)expr);
            case ast_varassign: return verifyVarAssign((VariableAssignAST*const&)expr);
            case ast_array_access: return verifyVarArrayAccess((VariableArrayAccessAST*const&)expr);

            case ast_type_access: return verifyTypeAccess((TypeAccessAST*const&)expr);

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