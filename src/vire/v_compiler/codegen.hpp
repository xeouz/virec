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
    llvm::LLVMContext CTX;
    llvm::IRBuilder<> Builder;
    std::unique_ptr<llvm::Module> Module;

    std::map<llvm::StringRef, llvm::AllocaInst*> namedValues;
    llvm::Function* currentFunction;
    llvm::BasicBlock* currentFunctionEndBB;
public:
    VCompiler(std::unique_ptr<VAnalyzer> analyzer) 
    : analyzer(std::move(analyzer)), Builder(llvm::IRBuilder<>(CTX))
    {
        Module = std::make_unique<llvm::Module>("vire", CTX);
    }
    
    llvm::Type* getLLVMType(const std::string& type);

    llvm::Value* compileExpr(ExprAST* const& expr);

    llvm::Value* compileNumExpr(IntExprAST* const& expr);
    llvm::Value* compileNumExpr(FloatExprAST* const& expr);
    llvm::Value* compileNumExpr(DoubleExprAST* const& expr);
    llvm::Value* compileCharExpr(CharExprAST* const& expr);
    llvm::Value* compileStrExpr(StrExprAST* const& expr);

    llvm::Value* compileBinopExpr(BinaryExprAST* const& expr);

    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function, std::string const& varname, llvm::Type* type);
    llvm::Value* compileVariableExpr(VariableExprAST* const& expr);
    llvm::Value* compileVariableDef(VariableDefAST* const& var);
    llvm::Value* compileVariableAssign(VariableAssignAST* const& var);

    std::vector<llvm::Value*> compileBlock(std::vector<std::unique_ptr<ExprAST>> const& block);

    llvm::Value* compileIfThen(IfThenExpr* const& ifthen);
    llvm::Value* compileIfElse(IfExprAST* const& ifelse);

    llvm::Value* compileCallExpr(CallExprAST* const& expr);
    llvm::Value* compileReturnExpr(ReturnExprAST* const& expr);
    llvm::Function* compilePrototype(std::string const& Name);
    llvm::Function* compileExtern(std::string const& Name);
    llvm::Function* compileFunction(std::string const& Name);
};
} // namespace vire