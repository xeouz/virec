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

namespace vire
{
class VCompiler
{
    std::unique_ptr<VAnalyzer> analyzer;

    // LLVM
public:


};
}