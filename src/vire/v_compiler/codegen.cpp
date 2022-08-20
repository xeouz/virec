#include "codegen.hpp"

namespace vire
{
    llvm::Type* VCompiler::getLLVMType(types::Base* type)
    {
        switch(type->getType())
        {
            case types::TypeNames::Void:
                return llvm::Type::getVoidTy(CTX);
            case types::TypeNames::Bool:
                return llvm::Type::getInt1Ty(CTX);
            case types::TypeNames::Char:
                return llvm::Type::getInt8Ty(CTX);
            case types::TypeNames::Int:
                return llvm::Type::getInt32Ty(CTX);
            case types::TypeNames::Float:
                return llvm::Type::getFloatTy(CTX);
            case types::TypeNames::Array:
                return llvm::ArrayType::get
                (getLLVMType(((types::Array*)type)->getChild()), ((types::Array*)type)->getLength());
            
            default:
                return llvm::Type::getVoidTy(CTX);
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
    llvm::Value* VCompiler::getAsPtrForGEP(llvm::Value* expr)
    {
        // Check if expr is llvm::LoadInst
        if (llvm::LoadInst* load = llvm::dyn_cast<llvm::LoadInst>(expr))
        {
            // remove the expr from the block
            load->getParent()->getInstList().remove(load);
            // return the alloca
            return namedValues[load->getPointerOperand()->getName()];
        }
        else
        {
            return expr;
        }
    }

    llvm::Value* VCompiler::compileExpr(ExprAST* const& expr)
    {
        if(!expr)   { return nullptr; }
        switch(expr->asttype)
        {
            case ast_int:
                return compileConstantExpr((IntExprAST* const&)expr);
            case ast_float:
                return compileConstantExpr((FloatExprAST* const&)expr);
            case ast_double:
                return compileConstantExpr((DoubleExprAST* const&)expr);
            case ast_char:
                return compileConstantExpr((CharExprAST* const&)expr);
            case ast_array:
                return compileConstantExpr((ArrayExprAST* const&)expr);

            case ast_varincrdecr:
                return compileIncrDecr((VariableIncrDecrAST* const&)expr);
            case ast_var:
                return compileVariableExpr((VariableExprAST* const&)expr);
            case ast_vardef:
                return compileVariableDef((VariableDefAST* const&)expr);
            case ast_varassign:
                return compileVariableAssign((VariableAssignAST* const&)expr);
            case ast_array_access:
                return compileVariableArrayAccess((VariableArrayAccessAST* const&)expr);

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
            case ast_continue:
                return compileContinueExpr((ContinueExprAST* const&)expr);

            case ast_return:
                return compileReturnExpr((ReturnExprAST* const&)expr);
            default:
                return nullptr;
        }
    }

    llvm::Constant* VCompiler::compileConstantExpr(ExprAST* const& expr)
    {
        switch(expr->getType()->getType())
        {
            case types::TypeNames::Int:
                return compileConstantExpr((IntExprAST* const&)expr);
            case types::TypeNames::Float:
                return compileConstantExpr((FloatExprAST* const&)expr);
            case types::TypeNames::Double:
                return compileConstantExpr((DoubleExprAST* const&)expr);
            case types::TypeNames::Char:
                return compileConstantExpr((CharExprAST* const&)expr);
            case types::TypeNames::Array:
                return compileConstantExpr((ArrayExprAST* const&)expr);
            default:
                return nullptr;
        }
    }
    llvm::Constant* VCompiler::compileConstantExpr(IntExprAST* const& expr)
    {
        
        return llvm::ConstantInt::get(CTX, llvm::APInt(32, expr->getValue(), true));
    }
    llvm::Constant* VCompiler::compileConstantExpr(FloatExprAST* const& expr)
    {
        return llvm::ConstantFP::get(CTX, llvm::APFloat(expr->getValue()));
    }
    llvm::Constant* VCompiler::compileConstantExpr(DoubleExprAST* const& expr)
    {
        return llvm::ConstantFP::get(CTX, llvm::APFloat(expr->getValue()));
    }
    llvm::Constant* VCompiler::compileConstantExpr(CharExprAST* const& expr)
    {
        return llvm::ConstantInt::get(CTX, llvm::APInt(8, expr->getValue(), true));
    }
    llvm::Constant* VCompiler::compileConstantExpr(StrExprAST* const& expr)
    {
        return nullptr;
    }

    llvm::Constant* VCompiler::compileConstantExpr(ArrayExprAST* const& expr, bool create_global_variable)
    {
        auto* type=getLLVMType(expr->getType());

        std::vector<llvm::Constant*> constants;
        for(auto& elem : expr->getElements())
        {
            llvm::Constant* constant;
            if(elem->getType()->getType() == types::TypeNames::Array)
            {
                constant=compileConstantExpr((ArrayExprAST* const&)elem, false);
            }
            else
            {
                constant=compileConstantExpr((ExprAST* const&)elem);
            }
            constants.push_back(constant);
        }

        auto* atype=llvm::ArrayType::get(type->getArrayElementType(), constants.size());

        if(!create_global_variable)
        {
            return llvm::ConstantArray::get(atype, constants);
        }

        constexpr auto linkage=llvm::GlobalValue::LinkageTypes::PrivateLinkage;
        auto* gbl=new llvm::GlobalVariable(type, true, linkage, nullptr, "array");
        gbl->setInitializer(llvm::ConstantArray::get(atype, constants));
        gbl->setUnnamedAddr(llvm::GlobalValue::UnnamedAddr::Global);
        gbl->setAlignment(llvm::Align(4));
        Module->getGlobalList().push_back(gbl);

        return gbl;
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

        if(!value)
        {
            namedValues[def->getName()]=alloca;
            return alloca;
        }

        bool is_array=(value->asttype==ast_array);
        if(!is_array)
        {
            auto* val=compileExpr(value);
            Builder.CreateStore(val, alloca);
        }
        else
        {
            auto* val=compileConstantExpr((ArrayExprAST* const&)value);

            // Create an i8*
            auto* ptr=Builder.CreateBitCast(alloca, llvm::IntegerType::getInt8PtrTy(CTX));

            // Call memcpy to copy the array
            auto align=alloca->getAlign();

            // Set the size of the array
            auto* array_ast=(ArrayExprAST* const&)value;
            auto elems=array_ast->getElements().size();
            auto elem_size=array_ast->getElements()[0]->getType()->getSize();
            auto size=elems*elem_size;

            // Create the memcpy call
            auto* call_inst=Builder.CreateMemCpy(ptr, align, val, align, llvm::ConstantInt::get(CTX, llvm::APInt(32, size, true)));
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
    llvm::Value* VCompiler::compileVariableArrayAccess(VariableArrayAccessAST* const& access)
    {
        /* Single index access, kept for reference */
        /*
            auto* var=namedValues[access->getName()];
            auto* indx_expr=(IntExprAST*)access->getIndex();
            int indx=indx_expr->getValue();

            // get the type of the array element
            auto* ty=var->getAllocatedType();

            llvm::Value* indx_val=llvm::ConstantInt::get(CTX, llvm::APInt(32, indx, false));

            llvm::Value* indices[2];
            indices[0]=llvm::ConstantInt::get(CTX, llvm::APInt(32, 0, false));  // index 0 is the array index
            indices[1]=indx_val;                                               // index 1 is the element index
            llvm::Value* indx_ptr=Builder.CreateInBoundsGEP(ty, var, llvm::ArrayRef<llvm::Value*>(indices, 2));
            return Builder.CreateLoad(ty->getArrayElementType(), indx_ptr);
        */

        /* Multi index access */
        auto* expr=compileExpr(access->getExpr());
        auto* ty=expr->getType();

        expr=getAsPtrForGEP(expr);

        for(auto const& elem : access->getIndices())
        {
            // Implemented only for int expressions, WIP
            if(elem->getType()->getType() != types::TypeNames::Int)
            {
                throw std::runtime_error("Array index must be an int expression, WIP");
            }
            
            auto* indx_expr=(IntExprAST*)elem.get();
            int indx=indx_expr->getValue();

            llvm::Value* indx_val=llvm::ConstantInt::get(CTX, llvm::APInt(32, indx, false));
            expr=Builder.CreateInBoundsGEP(ty, expr, {llvm::ConstantInt::get(CTX, llvm::APInt(32, 0, false)), indx_val});
            
            ty=ty->getArrayElementType();
        }

        return Builder.CreateLoad(ty, expr);
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
        currentLoopBodyBB=forloop;

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
        currentLoopBodyBB=whileloop;

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
    llvm::Value* VCompiler::compileContinueExpr(ContinueExprAST* const& continueexpr)
    {
        compileExpr(continueexpr->getAfterCont());

        return Builder.CreateBr(currentLoopBodyBB);
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

        llvm::Type* func_ret_type=getLLVMType(proto->getReturnType());

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
        // func->print(llvm::errs());
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
        auto ret_type=getLLVMType(func->getReturnType());
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

        return function;
    }

    llvm::Module* VCompiler::getModule()
    {
        return Module.get();
    }
    std::string VCompiler::getCompiledOutput()
    {
        std::string code;
        llvm::raw_string_ostream os(code);
        Module->print(os, nullptr);
        return code;
    }

    void VCompiler::compileToObjectFile(const std::string& filename)
    {
        std::string target_triple=llvm::sys::getDefaultTargetTriple();

        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();    

        std::string error;
        auto target=llvm::TargetRegistry::lookupTarget(target_triple, error);
        if(!target)
        {
            llvm::errs() << "Target not found:\n" << error;
            return;
        }

        auto cpu=llvm::sys::getHostCPUName();
        auto features="";

        llvm::TargetOptions opt;
        auto rm=llvm::Optional<llvm::Reloc::Model>();
        auto target_machine=target->createTargetMachine(target_triple, cpu, features, opt, rm);

        Module->setDataLayout(target_machine->createDataLayout());
        Module->setTargetTriple(target_triple);

        llvm::legacy::PassManager pass_manager;
        std::error_code ec;
        llvm::raw_fd_ostream os(filename, ec, llvm::sys::fs::OF_None);

        llvm::legacy::PassManager pass;
        auto file_type=llvm::CGFT_ObjectFile;
        target_machine->addPassesToEmitFile(pass, os, nullptr, file_type);

        pass.run(*Module);
        os.flush();
    }
}
