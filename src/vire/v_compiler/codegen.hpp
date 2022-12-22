#pragma once

#include "vire/ast/include.hpp"
#include "vire/v_analyzer/include.hpp"

// For `VIRE_ENABLE_ONLY` definition
#include "vire/config/config.hpp"

#ifdef VIRE_ENABLE_ONLY
    #define __SPECIFIC_INIT_MACRO(target, func_name) LLVMInitialize##target##func_name()
    #define SPECIFIC_INIT_TARGET_INFO(target) __SPECIFIC_INIT_MACRO(target, TargetInfo)
    #define SPECIFIC_INIT_TARGET(target) __SPECIFIC_INIT_MACRO(target, Target)
    #define SPECIFIC_INIT_TARGET_MC(target) __SPECIFIC_INIT_MACRO(target, TargetMC)
    #define SPECIFIC_INIT_ASM_PARSER(target) __SPECIFIC_INIT_MACRO(target, AsmParser)
    #define SPECIFIC_INIT_ASM_PRINTER(target) __SPECIFIC_INIT_MACRO(target, AsmPrinter)
#endif

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
#include "llvm/Transforms/InstCombine/InstCombine.h"
#include "llvm/Transforms/Scalar.h"
#include "llvm/Transforms/Scalar/GVN.h"
#include "llvm/Transforms/Utils.h"
#include "llvm/Analysis/LoopAnalysisManager.h"
#include "llvm/Analysis/CGSCCPassManager.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/IR/PassManager.h"
#include "llvm/Analysis/AliasAnalysis.h"
#include "llvm/Transforms/Scalar/DeadStoreElimination.h"

#include <memory>
#include <map>
#include <string>

namespace vire
{

enum class Optimization
{
    O0,
    O1,
    O2,
    O3,
    Os,
    Oz,
};

class VCompiler
{
    std::unique_ptr<VAnalyzer> analyzer;

    // LLVM
    llvm::LLVMContext CTX;
    llvm::IRBuilder<> Builder;
    std::unique_ptr<llvm::Module> Module;

    // Memory
    std::map<llvm::StringRef, llvm::AllocaInst*> namedValues;
    std::map<std::string, llvm::StructType*> definedStructs;
    llvm::Function* currentFunction;
    llvm::BasicBlock* currentFunctionEndBB;
    llvm::BasicBlock* currentLoopEndBB;
    llvm::BasicBlock* currentLoopBodyBB;
    FunctionAST* currentFunctionAST;
    bool current_func_single_sret;

    // Compilation
    enum llvm::CodeGenFileType file_type;
    std::string output_ir;
private:
    llvm::TargetMachine* compileInternal(std::string const& target_str);
    void runOptimizationPasses(llvm::TargetMachine* tm, Optimization opt_level=Optimization::O0, bool enable_lto=false);

public:
    VCompiler(std::unique_ptr<VAnalyzer> analyzer, std::string const& name="vire")
    : analyzer(std::move(analyzer)), Builder(llvm::IRBuilder<>(CTX))
    {
        Module = std::make_unique<llvm::Module>(name, CTX);
        CTX.setOpaquePointers(true);
        file_type=llvm::CGFT_ObjectFile;
    }

    // Compilation Functions
    
    llvm::Type* getLLVMType(types::Base* type);

    llvm::Value* pushFrontToCallInst(llvm::Value* arg, llvm::CallInst* call);
    llvm::Value* createAllocaForVar(VariableDefAST* const& var);
    llvm::Value* createBinaryOperation(llvm::Value* lhs, llvm::Value* rhs, VToken* const op, bool expr_is_fp);
    llvm::BranchInst* createBrIfNoTerminator(llvm::BasicBlock* block);
    llvm::Value* getValueAsAlloca(llvm::Value* value);
    llvm::Value* getOrigin(llvm::Value* value);

    llvm::Value* compileExpr(ExprAST* const expr);

    llvm::Constant* compileConstantExpr(ExprAST* const expr);
    llvm::Constant* compileConstantExpr(IntExprAST* const expr);
    llvm::Constant* compileConstantExpr(FloatExprAST* const expr);
    llvm::Constant* compileConstantExpr(DoubleExprAST* const expr);
    llvm::Constant* compileConstantExpr(CharExprAST* const expr);
    llvm::Constant* compileConstantExpr(StrExprAST* const expr);
    llvm::Constant* compileConstantExpr(BoolExprAST* const expr);
    llvm::Constant* compileConstantExpr(ArrayExprAST* const expr, bool create_global_variable=true);

    llvm::Value* compileBinopExpr(BinaryExprAST* const expr);

    llvm::AllocaInst* createEntryBlockAlloca(llvm::Function* function, std::string const& varname, llvm::Type* type);
    
    llvm::Value* compileIncrementDecrement(IncrementDecrementAST* const expr);
    llvm::Value* compileVariableExpr(VariableExprAST* const expr);
    llvm::Value* compileVariableDef(VariableDefAST* const var);
    llvm::Value* compileVariableAssign(VariableAssignAST* const var);
    llvm::Value* compileVariableArrayAccess(VariableArrayAccessAST* const var);
    llvm::Value* compileCastExpr(CastExprAST* const var);

    std::vector<llvm::Value*> compileBlock(std::vector<std::unique_ptr<ExprAST>> const& block);

    llvm::Value* compileIfThen(IfThenExpr* const ifthen);
    llvm::Value* compileIfElse(IfExprAST* const ifelse);

    llvm::Value* compileForExpr(ForExprAST* const forexpr);
    llvm::Value* compileWhileExpr(WhileExprAST* const whileexpr);
    llvm::Value* compileBreakExpr(BreakExprAST* const breakexpr);
    llvm::Value* compileContinueExpr(ContinueExprAST* const continueexpr);

    llvm::Value* compileCallExpr(CallExprAST* const expr);
    llvm::Value* compileReturnExpr(ReturnExprAST* const expr);
    llvm::Function* compilePrototype(PrototypeAST* const name);
    llvm::Function* compileExtern(std::string const& name);
    llvm::Function* compileFunction(std::string const& name);

    llvm::StructType* compileUnion(std::string const& name);
    llvm::StructType* compileStruct(std::string const& name, StructExprAST* st=nullptr);
    llvm::Value* compileTypeAccess(TypeAccessAST* const name);

    llvm::Module* const getModule() const;
    std::string const& getCompiledOutput();
    VAnalyzer* const getAnalyzer()  const;
    
    void resetModule();
    void compileModule();
    void compileToFile(std::string const& filename, std::string const& target, Optimization opt_level=Optimization::O0, bool enable_lto=false);
    std::vector<unsigned char> compileToString(std::string const& target_str="", Optimization opt_level=Optimization::O0, bool enable_lto=false);
};
} // namespace vire