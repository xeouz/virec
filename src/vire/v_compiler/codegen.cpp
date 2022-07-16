#include "codegen.hpp"

namespace vire
{
    llvm::Type* VCompiler::getLLVMType(const std::string& type)
    {
        if (type == "int")
        {
            return llvm::Type::getInt32Ty(CTX);
        }
        else if (type == "float")
        {
            return llvm::Type::getFloatTy(CTX);
        }
        else if (type == "double")
        {
            return llvm::Type::getDoubleTy(CTX);
        }
        else if (type == "char")
        {
            return llvm::Type::getInt8Ty(CTX);
        }
        else if (type == "str")
        {
            return llvm::Type::getInt8PtrTy(CTX);
        }
        else
        {
            return nullptr;
        }
    }

    llvm::BranchInst* VCompiler::createBrIfNoTerminator(llvm::BasicBlock* block)
    {
        if (Builder.GetInsertBlock()->getTerminator() == nullptr)
        {
            return Builder.CreateBr(block);
        }
        else
        {
            return nullptr;
        }
    }

    llvm::Value* VCompiler::compileExpr(ExprAST* const& expr)
    {
        if(!expr)   { return nullptr; }
        switch(expr->asttype)
        {
            case ast_int:
                return compileNumExpr((IntExprAST* const&)expr);
            case ast_float:
                return compileNumExpr((FloatExprAST* const&)expr);
            case ast_double:
                return compileNumExpr((DoubleExprAST* const&)expr);
            case ast_char:
                return compileCharExpr((CharExprAST* const&)expr);

            case ast_varincrdecr:
                return compileIncrDecr((VariableIncrDecrAST* const&)expr);
            case ast_var:
                return compileVariableExpr((VariableExprAST* const&)expr);
            case ast_vardef:
                return compileVariableDef((VariableDefAST* const&)expr);
            case ast_varassign:
                return compileVariableAssign((VariableAssignAST* const&)expr);

            case ast_binop:
                return compileBinopExpr((BinaryExprAST* const&)expr);

            case ast_call:
                return compileCallExpr((CallExprAST* const&)expr);

            case ast_ifelse:
                return compileIfElse((IfExprAST* const&)expr);
            
            case ast_for:
                return compileForExpr((ForExprAST* const&)expr);
            case ast_while:
                return compileWhileExpr((WhileExprAST* const&)expr);
            case ast_break:
                return compileBreakExpr((BreakExprAST* const&)expr);

            case ast_return:
                return compileReturnExpr((ReturnExprAST* const&)expr);
            default:
                return nullptr;
        }
    }

    llvm::Value* VCompiler::compileNumExpr(IntExprAST* const& expr)
    {
        
        return llvm::ConstantInt::get(CTX, llvm::APInt(32, expr->getValue(), true));
    }
    llvm::Value* VCompiler::compileNumExpr(FloatExprAST* const& expr)
    {
        return llvm::ConstantFP::get(CTX, llvm::APFloat(expr->getValue()));
    }
    llvm::Value* VCompiler::compileNumExpr(DoubleExprAST* const& expr)
    {
        return llvm::ConstantFP::get(CTX, llvm::APFloat(expr->getValue()));
    }
    llvm::Value* VCompiler::compileCharExpr(CharExprAST* const& expr)
    {
        return llvm::ConstantInt::get(CTX, llvm::APInt(8, expr->getValue(), true));
    }
    llvm::Value* VCompiler::compileStrExpr(StrExprAST* const& expr)
    {
        return Builder.CreateGlobalString(expr->getValue(), "str");
    }

    llvm::Value* VCompiler::compileIncrDecr(VariableIncrDecrAST* const& expr)
    {
        llvm::AllocaInst* val=namedValues[expr->getName()];
        llvm::Type* valtype=val->getAllocatedType();
        llvm::LoadInst* load=Builder.CreateLoad(valtype,val,expr->getName());

        // Currently only pre increment and decrement is supported.
        llvm::StoreInst* store;
        if(expr->isIncr())
        {
            store=Builder.CreateStore
            (Builder.CreateAdd(load, llvm::ConstantInt::get(valtype, 1)), val);
        }
        else
        {
            store=Builder.CreateStore
            (Builder.CreateSub(load, llvm::ConstantInt::get(valtype, 1)), val);
        }

        return load;
    }
    llvm::Value* VCompiler::compileVariableExpr(VariableExprAST* const& expr)
    {
        llvm::AllocaInst* val=namedValues[expr->getName()];
        llvm::Type* ty=val->getAllocatedType();
        return Builder.CreateLoad(ty, val, expr->getName());
    }
    llvm::Value* VCompiler::compileVariableDef(VariableDefAST* const& def)
    {
        auto* var_type=getLLVMType(def->getType());
        auto* current_blk=Builder.GetInsertBlock();
        
        auto* alloca=Builder.CreateAlloca(var_type, nullptr, def->getName());

        auto const& value=def->getValue();
        if(value)
        {
            auto* val=compileExpr(value);
            Builder.CreateStore(val, alloca);
        }

        namedValues[def->getName()]=alloca;

        return alloca;
    }
    llvm::Value* VCompiler::compileVariableAssign(VariableAssignAST* const& assign)
    {
        auto* var=namedValues[assign->getName()];
        auto* val=compileExpr(assign->getValue());

        Builder.CreateStore(val, var);

        return val;
    }

    llvm::Value* VCompiler::compileBinopExpr(BinaryExprAST* const& expr)
    {
        auto lhs = compileExpr(expr->getLHS());
        auto rhs = compileExpr(expr->getRHS());

        if(!lhs || !rhs)
            return nullptr;
        
        switch(expr->getOp()->type)
        {
            case tok_plus:
                return Builder.CreateAdd(lhs, rhs, "addtmp");
            case tok_minus:
                return Builder.CreateSub(lhs, rhs, "subtmp");
            case tok_mul:
                return Builder.CreateMul(lhs, rhs, "multmp");
            case tok_div:
                return Builder.CreateSDiv(lhs, rhs, "divtmp");
            
            case tok_lessthan:
                return Builder.CreateICmpSLT(lhs, rhs, "cmptmp");
            case tok_morethan:
                return Builder.CreateICmpSGT(lhs, rhs, "cmptmp");
            case tok_dequal:
                return Builder.CreateICmpEQ(lhs, rhs, "cmptmp");
            case tok_nequal:
                return Builder.CreateICmpNE(lhs, rhs, "cmptmp");
            
            default:
                std::cout << "Unhandled binary operator" << std::endl;
                return nullptr;
        }
    }

    std::vector<llvm::Value*> VCompiler::compileBlock(std::vector<std::unique_ptr<ExprAST>> const& block)
    {
        std::vector<llvm::Value*> values;
        for(const auto& expr : block)
        {
            values.push_back(compileExpr(expr.get()));
        }

        return values;
    }

    llvm::Value* VCompiler::compileIfThen(IfThenExpr* const& ifthen)
    {
        auto* cond=compileExpr(ifthen->getCondition());
        
        auto* iftrue=llvm::BasicBlock::Create(CTX, "ift", currentFunction);
        auto* ifcont=llvm::BasicBlock::Create(CTX, "ifc", currentFunction);
        auto* br=Builder.CreateCondBr(cond, iftrue, ifcont);

        Builder.SetInsertPoint(iftrue);
        compileBlock(ifthen->getThenBlock());
        createBrIfNoTerminator(ifcont);
        Builder.SetInsertPoint(ifcont);

        return br;
    }
    llvm::Value* VCompiler::compileIfElse(IfExprAST* const& ifelse)
    {
        auto* ifthen=compileIfThen(ifelse->getIfThen());

        for(const auto& elseif : ifelse->getElifLadder())
        {
            if(elseif->getCondition() != nullptr)
            {
                compileIfThen(elseif.get());
            }
            else
            {
                compileBlock(elseif->getThenBlock());
                break;
            }
        }

        return ifthen;
    }

    llvm::Value* VCompiler::compileForExpr(ForExprAST* const& forexpr)
    {
        auto* init=compileExpr(forexpr->getInit());

        auto* forbool=llvm::BasicBlock::Create(CTX, "forb", currentFunction);
        auto* forloop=llvm::BasicBlock::Create(CTX, "forl", currentFunction);
        auto* forcont=llvm::BasicBlock::Create(CTX, "forc", currentFunction);
        currentLoopEndBB=forcont;

        Builder.CreateBr(forbool);
        Builder.SetInsertPoint(forbool);
        auto* cond=compileExpr(forexpr->getCond());
        auto* incr=compileExpr(forexpr->getIncr());
        auto* br=Builder.CreateCondBr(cond, forloop, forcont);

        Builder.SetInsertPoint(forloop);
        compileBlock(forexpr->getBody());
        createBrIfNoTerminator(forbool);

        Builder.SetInsertPoint(forcont);
        return br;
    }
    llvm::Value* VCompiler::compileWhileExpr(WhileExprAST* const& whileexpr)
    {
        auto whilebool=llvm::BasicBlock::Create(CTX, "whileb", currentFunction);
        auto whileloop=llvm::BasicBlock::Create(CTX, "whilel", currentFunction);
        auto whilecont=llvm::BasicBlock::Create(CTX, "whilec", currentFunction);
        currentLoopEndBB=whilecont;

        Builder.CreateBr(whilebool);
        Builder.SetInsertPoint(whilebool);
        auto* cond=compileExpr(whileexpr->getCond());
        auto* br=Builder.CreateCondBr(cond, whileloop, whilecont);

        Builder.SetInsertPoint(whileloop);
        compileBlock(whileexpr->getBody());
        createBrIfNoTerminator(whilebool);

        Builder.SetInsertPoint(whilecont);

        return br;
    }
    llvm::Value* VCompiler::compileBreakExpr(BreakExprAST* const& breakexpr)
    {
        compileExpr(breakexpr->getAfterBreak());

        return Builder.CreateBr(currentLoopEndBB);
    }

    llvm::Value* VCompiler::compileCallExpr(CallExprAST* const& expr)
    {
        auto func=Module->getFunction(expr->getName());

        std::vector<llvm::Value*> args;
        for(auto& arg : expr->getArgs())
        {
            args.push_back(compileExpr(arg.get()));
        }

        return Builder.CreateCall(func, args, "calltmp");
    }
    llvm::Value* VCompiler::compileReturnExpr(ReturnExprAST* const& expr)
    {
        auto* expr_val=compileExpr(expr->getValue());
        auto* value=Builder.CreateStore(expr_val,namedValues["retval"]);
        Builder.CreateBr(currentFunctionEndBB);

        return value;
    }
    llvm::Function* VCompiler::compilePrototype(const std::string& Name)
    {
        auto const& base_ast=analyzer->getFunc(Name);
        auto const& proto=(std::unique_ptr<PrototypeAST> const&)base_ast;
        auto const& proto_args=proto->getArgs();
        std::vector<llvm::Type*> args(proto_args.size());

        llvm::Type* func_ret_type=getLLVMType(proto->getType());

        for(size_t i=0; i<proto_args.size(); i++)
        {
            args[i]=getLLVMType(proto_args[i]->getType());
        }

        llvm::FunctionType* func_type=llvm::FunctionType::get(func_ret_type, args, false);
        llvm::Function* func=llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, Name, Module.get());

        unsigned idx=0;
        for(auto& arg: func->args())
        {
            arg.setName(proto_args[idx]->getName());
        }

        return func;
    }
    llvm::Function* VCompiler::compileExtern(const std::string& Name)
    {
        llvm::Function* func=compilePrototype(Name);

        // Remove in release
        std::cout << "Compiled extern " << Name << std::endl;
        func->print(llvm::errs());
        return func;
    }
    llvm::Function* VCompiler::compileFunction(const std::string& Name)
    {
        llvm::Function* function=Module->getFunction(Name);
        if(!function)
            function=compilePrototype(Name);

        llvm::BasicBlock* bb=llvm::BasicBlock::Create(CTX, "entry", function);
        currentFunctionEndBB=llvm::BasicBlock::Create(CTX, "end", function);
        Builder.SetInsertPoint(bb);

        namedValues.clear();
        for(auto& arg: function->args())
        {
            auto* alloca=Builder.CreateAlloca(arg.getType(), nullptr, arg.getName());
            Builder.CreateStore(&arg, alloca);
            namedValues[arg.getName()]=alloca;
        }

        auto const& func=(FunctionAST*)analyzer->getFunc(Name);

        // Create return value
        auto ret_type=getLLVMType(func->getType());
        auto ret_val=Builder.CreateAlloca(ret_type, nullptr, "retval");
        namedValues["retval"]=ret_val;

        // Compile the block
        currentFunction=function;
        compileBlock(func->getBody());
        createBrIfNoTerminator(currentFunctionEndBB);

        // Create the return instruction
        Builder.SetInsertPoint(currentFunctionEndBB);
        Builder.CreateRet(Builder.CreateLoad(ret_type, ret_val, "ret"));
        auto& bbend=function->getBasicBlockList().back();
        currentFunctionEndBB->moveAfter(&bbend); // Move the end block after the 
                                                 // last block of the function

        llvm::verifyFunction(*function);
        
        // Remove in release
        std::cout << "Compiled function " << Name << std::endl;
        function->print(llvm::errs());

        return function;
    }

}
