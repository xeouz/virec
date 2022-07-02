#include "codegen.hpp"

namespace vire
{
    llvm::Type* VCompiler::getLLVMType(const std::string& type)
    {
        if (type == "int")
        {
            return llvm::Type::getInt32Ty(ctx);
        }
        else if (type == "float")
        {
            return llvm::Type::getFloatTy(ctx);
        }
        else if (type == "double")
        {
            return llvm::Type::getDoubleTy(ctx);
        }
        else if (type == "char")
        {
            return llvm::Type::getInt8Ty(ctx);
        }
        else if (type == "str")
        {
            return llvm::Type::getInt8PtrTy(ctx);
        }
        else
        {
            return nullptr;
        }
    }

    llvm::Value* VCompiler::compileExpr(const std::unique_ptr<ExprAST>& expr)
    {
        switch(expr->asttype)
        {
            case ast_int:
                return compileNumExpr((std::unique_ptr<IntExprAST> const&)expr);
            case ast_float:
                return compileNumExpr((std::unique_ptr<FloatExprAST> const&)expr);
            case ast_double:
                return compileNumExpr((std::unique_ptr<DoubleExprAST> const&)expr);
            case ast_char:
                return compileCharExpr((std::unique_ptr<CharExprAST> const&)expr);

            case ast_var:
                return compileVariableExpr((std::unique_ptr<VariableExprAST> const&)expr);

            case ast_binop:
                return compileBinopExpr((std::unique_ptr<BinaryExprAST> const&)expr);

            case ast_call:
                return compileCallExpr((std::unique_ptr<CallExprAST> const&)expr);

            case ast_return:
                return compileReturnExpr((std::unique_ptr<ReturnExprAST> const&)expr);
            default:
                return nullptr;
        }
    }

    llvm::Value* VCompiler::compileNumExpr(const std::unique_ptr<IntExprAST>& expr)
    {
        
        return llvm::ConstantInt::get(ctx, llvm::APInt(32, expr->getValue(), true));
    }
    llvm::Value* VCompiler::compileNumExpr(const std::unique_ptr<FloatExprAST>& expr)
    {
        return llvm::ConstantFP::get(ctx, llvm::APFloat(expr->getValue()));
    }
    llvm::Value* VCompiler::compileNumExpr(const std::unique_ptr<DoubleExprAST>& expr)
    {
        return llvm::ConstantFP::get(ctx, llvm::APFloat(expr->getValue()));
    }
    llvm::Value* VCompiler::compileCharExpr(const std::unique_ptr<CharExprAST>& expr)
    {
        return llvm::ConstantInt::get(ctx, llvm::APInt(8, expr->getValue(), true));
    }
    llvm::Value* VCompiler::compileStrExpr(const std::unique_ptr<StrExprAST>& expr)
    {
        return builder.CreateGlobalString(expr->getValue(), "str");
    }

    llvm::Value* VCompiler::compileVariableExpr(const std::unique_ptr<VariableExprAST>& expr)
    {
        return namedValues[expr->getName()];
    }

    llvm::Value* VCompiler::compileBinopExpr(const std::unique_ptr<BinaryExprAST>& expr)
    {
        auto lhs = compileExpr(expr->getLHS());
        auto rhs = compileExpr(expr->getRHS());

        if(!lhs || !rhs)
            return nullptr;
        
        switch(expr->getOp()->type)
        {
            case tok_plus:
                return builder.CreateAdd(lhs, rhs, "addtmp");
            case tok_minus:
                return builder.CreateSub(lhs, rhs, "subtmp");
            case tok_mul:
                return builder.CreateMul(lhs, rhs, "multmp");
            case tok_div:
                return builder.CreateSDiv(lhs, rhs, "divtmp");
            
            case tok_lessthan:
                return builder.CreateICmpSLT(lhs, rhs, "cmptmp");
            case tok_morethan:
                return builder.CreateICmpSGT(lhs, rhs, "cmptmp");
            
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
            values.push_back(compileExpr(expr));
        }

        return values;
    }

    llvm::Value* VCompiler::compileCallExpr(const std::unique_ptr<CallExprAST>& expr)
    {
        auto func=Module->getFunction(expr->getName());

        std::vector<llvm::Value*> args;
        for(auto& arg : expr->getArgs())
        {
            args.push_back(compileExpr(arg));
        }

        return builder.CreateCall(func, args, "calltmp");
    }
    llvm::Value* VCompiler::compileReturnExpr(const std::unique_ptr<ReturnExprAST>& expr)
    {
        auto ret=builder.CreateRet(compileExpr(expr->getValues()[0]));
        builder.CreateUnreachable();
        return ret;
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
        std::cout << "Compiled extern " << Name << std::endl;
        func->print(llvm::errs());
        return func;
    }
    llvm::Function* VCompiler::compileFunction(const std::string& Name)
    {
        llvm::Function* function=Module->getFunction(Name);
        if(!function)
            function=compilePrototype(Name);

        llvm::BasicBlock* bb=llvm::BasicBlock::Create(ctx, "entry", function);
        builder.SetInsertPoint(bb);

        namedValues.clear();
        for(auto& arg: function->args())
        {
            namedValues[arg.getName()]=&arg;
        }

        auto const& func=(std::unique_ptr<FunctionAST> const&)analyzer->getFunc(Name);
        for(const auto& stm : func->getBody())
        {
            compileExpr(stm);
        }
        
        llvm::verifyFunction(*function);
        
        std::cout << "Compiled function " << Name << std::endl;
        function->print(llvm::errs());

        return function;
    }

}
