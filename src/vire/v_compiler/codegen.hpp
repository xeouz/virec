#pragma once

#include "../includes.hpp"
#include __VIRE_AST_PATH
#include __VIRE_VANALYZER_PATH

// LLVM
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"

#include <memory>
#include <map>
#include <string>

namespace vire
{
class VCompiler
{
    std::unique_ptr<VAnalyzer> analyzer;

    // LLVM
    llvm::LLVMContext ctx;
    llvm::IRBuilder<> builder;
    std::unique_ptr<llvm::Module> Module;

    std::map<llvm::StringRef, llvm::Value*> namedValues;
public:
    VCompiler(std::unique_ptr<VAnalyzer> analyzer) 
    : analyzer(std::move(analyzer)), builder(llvm::IRBuilder<>(ctx))
    {
        Module = std::make_unique<llvm::Module>("vire", ctx);
    }
    
    llvm::Type* getLLVMType(const std::string& type);

    llvm::Value* compileExpr(const std::unique_ptr<ExprAST>& expr);

    llvm::Value* compileNumExpr(const std::unique_ptr<IntExprAST>& expr);
    llvm::Value* compileNumExpr(const std::unique_ptr<FloatExprAST>& expr);
    llvm::Value* compileNumExpr(const std::unique_ptr<DoubleExprAST>& expr);
    llvm::Value* compileCharExpr(const std::unique_ptr<CharExprAST>& expr);
    llvm::Value* compileStrExpr(const std::unique_ptr<StrExprAST>& expr);

    llvm::Value* compileBinopExpr(const std::unique_ptr<BinaryExprAST>& expr);

    llvm::Value* compileVariableExpr(const std::unique_ptr<VariableExprAST>& expr);
    llvm::Value* compileVariableDef(const std::unique_ptr<VariableDefAST>& var);

    std::vector<llvm::Value*> compileBlock(std::vector<std::unique_ptr<ExprAST>> const& block);

    llvm::Value* compileCallExpr(const std::unique_ptr<CallExprAST>& expr);
    llvm::Value* compileReturnExpr(const std::unique_ptr<ReturnExprAST>& expr);
    llvm::Function* compilePrototype(const std::string& Name);
    llvm::Function* compileExtern(const std::string& Name);
    llvm::Function* compileFunction(const std::string& Name);
};
} // namespace vire