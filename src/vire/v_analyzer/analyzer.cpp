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

    bool VAnalyzer::isStructDefined(const std::string& name)
    {
        return types::isTypeinMap(name);
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

    void VAnalyzer::addVar(VariableDefAST* const& var)
    {
        scope.insert(std::make_pair(var->getName(), var));
        
        if(scope_varref != nullptr)
        {
            scope_varref->push_back(var);
        }
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

        if(name=="")
        {
            return current_func;
        }
        for(int i=0; i<functions.size(); i++)
        {
            if(functions[i]->getName()==name)
            {
                return functions[i].get();
            }
        }

        std::cout << "Function `" << name << "` not found" << std::endl;
        return nullptr;
    }
    types::Base* VAnalyzer::getFuncReturnType(const std::string& name)
    {
        auto* type=getFunc(name)->getReturnType();
        return type;
    }

    CodeAST* const VAnalyzer::getCode()
    {
        return codeast.get();
    }

    VariableDefAST* const VAnalyzer::getVar(const std::string& name)
    {
        if(!isVarDefined(name))
        {
           std::cout << "Variable `" << name << "` not found" << std::endl;
           return nullptr;
        }
        return scope.at(name);
    }
    StructExprAST* const VAnalyzer::getStruct(const std::string& name)
    {
        if(!isStructDefined(name))
        {
            return nullptr;
        }

        auto const& structs=codeast->getUnionStructs();
        for(auto const& expr : structs)
        {
            if(expr->asttype==ast_struct)
            {
                auto* st=(StructExprAST*)expr.get();

                if(st->getName()==name)
                {
                    return st;
                }
            }
        }

        return nullptr;
    }

    // !!- CHANGES REQUIRED -!!
    // str to be implemented
    types::Base* VAnalyzer::getType(ExprAST* const& expr)
    {
        switch (expr->asttype)
        {
            case ast_int: return expr->getType();
            case ast_float: return expr->getType();
            case ast_double: return expr->getType();
            case ast_char: return expr->getType();

            case ast_binop: return getType(((BinaryExprAST*)expr)->getLHS());

            case ast_varincrdecr: 
            return getVar(((VariableExprAST*)expr)->getName())->getType();
            case ast_var:
            {
                auto* var=getVar(((VariableExprAST*)expr)->getName());
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

            case ast_call: return getFuncReturnType(((CallExprAST*)expr)->getName());

            case ast_array: return getType((ArrayExprAST*)expr);

            default:
            {
                std::cout<<"Error: Unknown type in getType()"<<std::endl;
                return nullptr;
            }
        }
    }
    
    // !!- CHANGES REQUIRED -!!
    // unhandled dynamic array typing
    types::Base* VAnalyzer::getType(ArrayExprAST* const& array)
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
                std::cout << var->getName() << " is valid" << std::endl;
                addVar(var);
                return true;
            }

            bool is_auto=(type->getType()==types::TypeNames::Void);

            if(is_auto && is_var)
            {
                std::cout << "`any` type not implement yet" << std::endl;
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
                    if(!types::isUserDefined(type) && !types::isUserDefined(value_type))
                    {
                        auto src_type=types::copyType(value_type);
                        auto dst_type=types::copyType(type);
                        auto new_value=var->moveValue();
                        auto new_cast_value=std::make_unique<CastExprAST>(std::move(new_value), std::move(dst_type), true);
                        new_cast_value->setSourceType(std::move(src_type));

                        var->setValue(std::move(new_cast_value));
                    }
                    else
                    {
                        std::cout << "Error: Type mismatch: " << *type << " " << *value_type << std::endl;
                        return false;
                    } 
                }
            }
            else
            {
                auto value_type_uptr=types::copyType(value_type);

                var->getValue()->setType(std::move(value_type_uptr));
                var->setUseValueType(true);
            }
            
            addVar(var);
            
            return true;
        }
        std::cout << "Variable already defined" << std::endl;
        // Variable is already defined
        return false;
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

        auto* assign_type=getType(assign->getValue());

        if(types::isSame(var->getType(), assign_type))
        {
            // Type mismatch
            std::cout << "Type mismatch in assignment, types are: " << var->getType() << " and " << *assign_type << std::endl;
            return false;
        }
        
        std::cout << "Type mismatch in assignment, types are: " << var->getType() << " and " << *assign_type << std::endl;
        return true;
    }
    bool VAnalyzer::verifyVarArrayAccess(VariableArrayAccessAST* const& access)
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
                    std::cout << "Index: " << std::endl;
                    auto* index_type=getType(index.get());
                    if(index->asttype==ast_int)
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
                        std::cout << "Error: Array index is not an integer" << std::endl;
                        return false;
                    }

                    child_array_type=(types::Array*)child_array_type->getChild();
                }
            }
        }
        
        access->setType(type);

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

        if(!isFuncDefined(name))
        {
            // Function is not defined
            is_valid=false;
        }

        const auto& args=call->getArgs();
        const auto& func_args=getFunc(name)->getArgs();

        if(args.size() != func_args.size())
        {
            // Argument count mismatch
            is_valid=false;
        }

        for(unsigned int i=0; i<args.size(); ++i)
        {
            const auto& arg=args[i];

            if(!verifyExpr(arg.get()))
            {
                // Argument is not valid
                is_valid=false;
            }

            auto* argtype=getType(arg.get());
            if(!types::isSame(func_args[i]->getType(), argtype))
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
        
        auto* ret_expr_type=getType(ret->getValue());

        if(!verifyExpr(ret->getValue()))
        {
            // Return value is not valid
            return false;
        }
        
        if(!types::isSame(ret_type, ret_expr_type))
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
                if(!verifyStruct((StructExprAST*)expr))
                {
                    // Struct is not valid
                    is_valid=false;
                }
            }
            else if(expr->asttype==ast_union)
            {
                if(!verifyUnion((UnionExprAST*)expr))
                {
                    // Union is not valid
                    is_valid=false;
                }
            }
        }

        return is_valid;
    }
    bool VAnalyzer::verifyUnion(UnionExprAST* const& union_)
    {
        auto const& members=union_->getMembersValues();
        
        if(!verifyUnionStructBody(members))
        {
            // Union body is not valid
            return false;
        }

        return true;
    }
    bool VAnalyzer::verifyStruct(StructExprAST* const& struct_)
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

        if(!types::isTypeinMap(struct_->getName()))
        {
            types::addTypeToMap(struct_->getName());
            types::addTypeSizeToMap(struct_->getName(), size);
        }
        else
        {
            std::cout << "Struct already defined" << std::endl;
            is_valid=false;
        }

        return is_valid;
    }
    bool VAnalyzer::verifyTypeAccess(ClassAccessAST* const& access)
    {
        bool is_valid=true;
        auto* ptype=getType(access->getParent());

        if(ptype->getType() != types::TypeNames::Custom)
        {
            std::cout << "Parent is not a type" << std::endl;
            is_valid=false;
        }

        auto* ptype_custom=(types::Custom*)ptype;

        if(!types::isTypeinMap(ptype_custom->getName()))
        {
            std::cout << "Type " << ptype_custom << " is not defined" << std::endl;
        }

        auto* st=getStruct(ptype_custom->getName());

        auto name=access->getChild()->getName();
        if(!st->isMember(name))
        {
            std::cout << "No member as " << name << " in struct" << std::endl;
            is_valid=false;
        }

        return is_valid;
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
            removeVar(var);
        }

        this->scope_varref=nullptr;
        delete refscope;
        
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

            codeast->addUnionStruct(std::move(union_structs[it]));
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
            case ast_varassign: return verifyVarAssign((VariableAssignAST*const&)expr);
            case ast_array_access: return verifyVarArrayAccess((VariableArrayAccessAST*const&)expr);

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