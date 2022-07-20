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
#include "llvm/Support/raw_ostream.h"
#include "llvm/Support/Host.h"
#include "llvm/IR/LegacyPassManager.h"
#include "llvm/MC/TargetRegistry.h"
#include "llvm/Support/TargetSelect.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Support/FileSystem.h"
#include "llvm/Target/TargetOptions.h"

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

    // Memory
    std::map<llvm::StringRef, llvm::AllocaInst*> namedValues;
    llvm::Function* currentFunction;
    llvm::BasicBlock* currentFunctionEndBB;
    llvm::BasicBlock* currentLoopEndBB;
    llvm::BasicBlock* currentLoopBodyBB;
public:
    VCompiler(std::unique_ptr<VAnalyzer> analyzer, std::string const& name="vire")
    : analyzer(std::move(analyzer)), Builder(llvm::IRBuilder<>(CTX))
    {
        Module = std::make_unique<llvm::Module>(name, CTX);
    }
    
    llvm::Type* getLLVMType(const std::string& type);

    llvm::BranchInst* createBrIfNoTerminator(llvm::BasicBlock* block);

    llvm::Value* compileExpr(ExprAST* const& expr);

    llvm::Value* compileNumExpr(IntExprAST* const& expr);
    llvm::Value* compileNumExpr(FloatExprAST* const& expr);
    llvm::Value* compileNumExpr(DoubleExprAST* const& expr);
    llvm::Value* compileCharExpr(CharExprAST* const& expr);
    llvm::Value* compileStrExpr(StrExprAST* const& expr);

    llvm::Value* compileBinopExpr(BinaryExprAST* const& expr);

    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function, std::string const& varname, llvm::Type* type);
    
    llvm::Value* compileIncrDecr(VariableIncrDecrAST* const& expr);
    llvm::Value* compileVariableExpr(VariableExprAST* const& expr);
    llvm::Value* compileVariableDef(VariableDefAST* const& var);
    llvm::Value* compileVariableAssign(VariableAssignAST* const& var);

    std::vector<llvm::Value*> compileBlock(std::vector<std::unique_ptr<ExprAST>> const& block);

    llvm::Value* compileIfThen(IfThenExpr* const& ifthen);
    llvm::Value* compileIfElse(IfExprAST* const& ifelse);

    llvm::Value* compileForExpr(ForExprAST* const& forexpr);
    llvm::Value* compileWhileExpr(WhileExprAST* const& whileexpr);
    llvm::Value* compileBreakExpr(BreakExprAST* const& breakexpr);
    llvm::Value* compileContinueExpr(ContinueExprAST* const& continueexpr);

    llvm::Value* compileCallExpr(CallExprAST* const& expr);
    llvm::Value* compileReturnExpr(ReturnExprAST* const& expr);
    llvm::Function* compilePrototype(std::string const& Name);
    llvm::Function* compileExtern(std::string const& Name);
    llvm::Function* compileFunction(std::string const& Name);

    llvm::Module* getModule();
    std::string getCompiledOutput();
    
    void compileToObjectFile(std::string const& filename);
};
} // namespace vire