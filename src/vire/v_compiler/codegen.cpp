#include "codegen.hpp"

// LLVM
#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/Attributes.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
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

#ifndef VIRE_NO_PASSES
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
#include "llvm/Support/JSON.h"
#endif

//-- CHANGES REQUIRED: STRUCT PACKING --//

namespace vire
{
    llvm::Type* VCompiler::getLLVMType(types::Base* type, bool allow_opaque_ptr)
    {
        switch(type->getType())
        {
            case types::EType::Void:    return llvm::Type::getVoidTy(CTX);
            case types::EType::Bool:    return llvm::Type::getInt1Ty(CTX);
            case types::EType::Char:    return llvm::Type::getInt8Ty(CTX);
            case types::EType::Short:   return llvm::Type::getInt16Ty(CTX);
            case types::EType::Int:     return llvm::Type::getInt32Ty(CTX);
            case types::EType::Long:    return llvm::Type::getInt64Ty(CTX);
            case types::EType::Float:   return llvm::Type::getFloatTy(CTX);
            case types::EType::Double:  return llvm::Type::getDoubleTy(CTX);
            case types::EType::Array:
            {
                auto* array=(types::Array*)type;
                
                return llvm::ArrayType::get(getLLVMType(array->getChild()), array->getLength());
            }
            
            case types::EType::Custom:
            {
                auto* custom=(types::Custom*)type;
                auto* ty=definedStructs[custom->getName()];

                if(allow_opaque_ptr)
                    return llvm::PointerType::get(ty, 0);
                
                return ty;
            }

            default:
                std::cout << "Unknown type: " << (int)type->getType() << std::endl;
                return llvm::Type::getVoidTy(CTX);
        }
    }

    void VCompiler::createSRetMemCpyForArg(ReturnExprAST* ret)
    {
        auto* ty=getLLVMType(ret->getValue()->getType());

        auto* src=compileExpr(ret->getValue());
        auto* dest=currentFunction->getArg(0);
        auto align=data_layout->getStructLayout((llvm::StructType*)ty)->getAlignment();

        long nsize=ret->getValue()->getType()->getSize();

        Builder.CreateMemCpy(dest, align, src, align, nsize);
    }
    llvm::CallInst* VCompiler::pushFrontToCallInst(llvm::Value* arg, llvm::CallInst* call)
    {
        std::vector<llvm::Value*> new_args;
        new_args.push_back(arg); // The alloca is the first argument
        for(auto const& arg: call->args()) // Add the rest of the arguments
        {
            new_args.push_back(arg);
        }

        // Replace the old call with the new call
        auto* ncall=Builder.CreateCall(call->getCalledFunction(), new_args);

        call->eraseFromParent();

        return ncall;
    }
    llvm::Value* VCompiler::createAllocaForVar(VariableDefAST* const& var)
    {
        auto* ty=getLLVMType(var->getType(), false);
        auto* alloca=Builder.CreateAlloca(ty, nullptr, var->getName());
        namedValues[var->getName()]=alloca;
        return alloca;
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
    llvm::Value* VCompiler::getValueAsAlloca(llvm::Value* expr)
    {
        // Checks
        if(llvm::AllocaInst* alloc=llvm::dyn_cast<llvm::AllocaInst>(expr))
        {
            return alloc;
        }
        else if (llvm::LoadInst* load=llvm::dyn_cast<llvm::LoadInst>(expr))
        {
            // Get the alloca by the name in the load operation
            auto* alloca=namedValues[load->getPointerOperand()->getName()];
            
            // remove the expr from the block
            load->eraseFromParent();

            return alloca;
        }
        else if(llvm::GetElementPtrInst* gep=llvm::dyn_cast<llvm::GetElementPtrInst>(expr))
        {
            // Get the alloca by the name in the GEP operation
            auto* alloca=namedValues[gep->getPointerOperand()->getName()];
            
            // remove the expr from the block
            gep->eraseFromParent();

            return alloca;
        }
        else
        {
            std::cout << "Could not cast expr to llvm::AllocaInst*:" << std::endl;
            llvm::errs() << "\t";
            expr->print(llvm::errs());
            llvm::errs() << "\n";
            return nullptr;
        }
    }
    
    //-- CHANGES REQUIRED: NUW and NSW flag toggling --//
    llvm::Value* VCompiler::createBinaryOperation(llvm::Value* lhs, llvm::Value* rhs, VToken* const op, bool expr_is_fp)
    {
        const char* cmpname="cmptmp";
        switch(op->type)
        {
            case tok_plus:
            {
                if(!expr_is_fp)
                {
                    return Builder.CreateAdd(lhs, rhs, "addtmp", false, true);
                }
                else
                {
                    return Builder.CreateFAdd(lhs, rhs, "addtmp");
                }
            }
            case tok_minus:
            {
                if(!expr_is_fp)
                {
                    return Builder.CreateSub(lhs, rhs, "subtmp", false, true);
                }
                else
                {
                    return Builder.CreateFSub(lhs, rhs, "subtmp");
                }
            }
            case tok_mul:
            {
                if(!expr_is_fp)
                {
                    return Builder.CreateMul(lhs, rhs, "multmp", false, true);
                }
                else
                {
                    return Builder.CreateFMul(lhs, rhs, "multmp");
                }
            }
            case tok_div:
            {
                if(!expr_is_fp)
                {
                    return Builder.CreateSDiv(lhs, rhs, "divtmp");
                }
                else
                {
                    return Builder.CreateFDiv(lhs, rhs, "divtmp");
                }
            }
            case tok_mod:
                return Builder.CreateSRem(lhs, rhs, cmpname);
            
            case tok_lessthan:
            {
                if(expr_is_fp)
                    return Builder.CreateFCmpOLT(lhs, rhs, cmpname);
                return Builder.CreateICmpSLT(lhs, rhs, cmpname);
            }
            case tok_morethan:
            {
                if(expr_is_fp)
                    return Builder.CreateFCmpOGT(lhs, rhs, cmpname);
                return Builder.CreateICmpSGT(lhs, rhs, cmpname);
            }
            case tok_dequal:
            {
                if(expr_is_fp)
                    return Builder.CreateFCmpOEQ(lhs, rhs, cmpname);
                return Builder.CreateICmpEQ(lhs, rhs, cmpname);
            }
            case tok_nequal:
            {
                if(expr_is_fp)
                    return Builder.CreateFCmpONE(lhs, rhs, cmpname);
                return Builder.CreateICmpNE(lhs, rhs, cmpname);
            }
            
            case tok_moreeq:
            {
                if(expr_is_fp)
                    return Builder.CreateFCmpOGE(lhs, rhs, cmpname);
                return Builder.CreateICmpSGE(lhs, rhs, cmpname);
            }
            case tok_lesseq:
            {
                if(expr_is_fp)
                    return Builder.CreateFCmpOLE(lhs, rhs, cmpname);
                return Builder.CreateICmpSLE(lhs, rhs, cmpname);
            }

            default:
                std::cout << "Unhandled binary operator" << std::endl;
                return nullptr;
        }
    }

    llvm::Value* VCompiler::compileExpr(ExprAST* const expr)
    {
        if(!expr)   { std::cout << "Invalid Expression" << std::endl;return nullptr; }
        switch(expr->asttype)
        {
            case ast_int:
                return compileConstantExpr((IntExprAST* const)expr);
            case ast_float:
                return compileConstantExpr((FloatExprAST* const)expr);
            case ast_double:
                return compileConstantExpr((DoubleExprAST* const)expr);
            case ast_char:
                return compileConstantExpr((CharExprAST* const)expr);
            case ast_bool: 
                return compileConstantExpr((BoolExprAST* const)expr);
            case ast_array:
                return compileConstantExpr((ArrayExprAST* const)expr);

            case ast_incrdecr:
                return compileIncrementDecrement((IncrementDecrementAST* const)expr);
            case ast_var:
                return compileVariable((VariableExprAST* const)expr);
            case ast_vardef:
                return compileVariableDefinition((VariableDefAST* const)expr);
            case ast_varassign:
                return compileVariableAssign((VariableAssignAST* const)expr);
            case ast_array_access:
                return compileVariableArrayAccess((VariableArrayAccessAST* const)expr);
            case ast_cast:
                return compileCastExpr((CastExprAST* const)expr);

            case ast_type_access:
                return compileTypeAccess((TypeAccessAST* const)expr);

            case ast_binop:
                return compileBinopExpr((BinaryExprAST* const)expr);

            case ast_call:
                return compileCallExpr((CallExprAST* const)expr);

            case ast_ifelse:
                return compileIfElse((IfExprAST* const)expr);
            
            case ast_for:
                return compileForExpr((ForExprAST* const)expr);
            case ast_while:
                return compileWhileExpr((WhileExprAST* const)expr);
            case ast_break:
                return compileBreakExpr((BreakExprAST* const)expr);
            case ast_continue:
                return compileContinueExpr((ContinueExprAST* const)expr);

            case ast_return:
                return compileReturnExpr((ReturnExprAST* const)expr);
            default:
                std::cout << "Unknown expression type: " << (int)expr->asttype << std::endl;
                return nullptr;
        }
    }

    llvm::Constant* VCompiler::compileConstantExpr(ExprAST* const expr)
    {
        switch(expr->getType()->getType())
        {
            case types::EType::Int:
                return compileConstantExpr((IntExprAST* const)expr);
            case types::EType::Float:
                return compileConstantExpr((FloatExprAST* const)expr);
            case types::EType::Double:
                return compileConstantExpr((DoubleExprAST* const)expr);
            case types::EType::Char:
                return compileConstantExpr((CharExprAST* const)expr);
            case types::EType::Array:
                return compileConstantExpr((ArrayExprAST* const)expr);
            default:
                return nullptr;
        }
    }
    llvm::Constant* VCompiler::compileConstantExpr(IntExprAST* const expr)
    {
        return llvm::ConstantInt::get(CTX, llvm::APInt(32, expr->getValue(), true));
    }
    llvm::Constant* VCompiler::compileConstantExpr(FloatExprAST* const expr)
    {
        return llvm::ConstantFP::get(CTX, llvm::APFloat(expr->getValue()));
    }
    llvm::Constant* VCompiler::compileConstantExpr(DoubleExprAST* const expr)
    {
        return llvm::ConstantFP::get(CTX, llvm::APFloat(expr->getValue()));
    }
    llvm::Constant* VCompiler::compileConstantExpr(CharExprAST* const expr)
    {
        return llvm::ConstantInt::get(CTX, llvm::APInt(8, expr->getValue(), true));
    }
    llvm::Constant* VCompiler::compileConstantExpr(StrExprAST* const expr)
    {
        return nullptr;
    }
    llvm::Constant* VCompiler::compileConstantExpr(BoolExprAST* const expr)
    {
        return llvm::ConstantInt::get(CTX, llvm::APInt(1, expr->getValue()));
    }
    llvm::Constant* VCompiler::compileConstantExpr(ArrayExprAST* const expr, bool create_global_variable)
    {
        auto* type=getLLVMType(expr->getType());

        std::vector<llvm::Constant*> constants;
        for(auto& elem : expr->getElements())
        {
            llvm::Constant* constant;
            if(elem->getType()->getType() == types::EType::Array)
            {
                constant=compileConstantExpr((ArrayExprAST* const)elem.get(), false);
            }
            else
            {
                constant=compileConstantExpr((ExprAST* const)elem.get());
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

    llvm::Value* VCompiler::compileIncrementDecrement(IncrementDecrementAST* const incrdecr)
    {
        auto const& target=incrdecr->getExpr();
        auto* expr=compileExpr(target);

        llvm::StoreInst* store;
        llvm::Value* inst;

        if(incrdecr->isIncrement())
        {
            if(!expr->getType()->isFloatingPointTy())
            {
                inst=Builder.CreateAdd(expr, llvm::ConstantInt::get(CTX, llvm::APInt(32, 1, false)), "", false, true);
            }
            else
            {
                inst=Builder.CreateFAdd(expr, llvm::ConstantFP::get(CTX, llvm::APFloat(1.0f)));
            }
        }
        else
        {
            if(!types::isTypeFloatingPoint(target->getType()))
            {
                inst=Builder.CreateSub(expr, llvm::ConstantInt::get(CTX, llvm::APInt(32, 1, false)));
            }
            else
            {
                inst=Builder.CreateFSub(expr, llvm::ConstantFP::get(CTX, llvm::APFloat(1.0f)));
            }
        }

        store=Builder.CreateStore(inst, getValueAsAlloca(compileExpr(target)));

        if(incrdecr->isPre())
        {
            return inst;
        }
        else
        {
            return expr;
        }
    }
    llvm::Value* VCompiler::compileVariable(VariableExprAST* const expr)
    { 
        auto* var=currentFunctionAST->getVariable(expr->getName());
        if(var->isReturned() && !var->isArgument() && current_func_single_sret)
        {
            return currentFunction->getArg(0);
        }

        llvm::Value* val;
        llvm::Type* ty;

        if(var->isArgument())
        {
            return currentFunction->getArg(currentFunctionAST->getArgumentIndex(var->getName()) + current_func_ret_ty);
        }
        else
        {
            auto* named=namedValues[expr->getName()];
            val=named;
            ty=named->getAllocatedType();
        }

        if(expr->getType()->getType()==types::EType::Array)
        {
            auto* alloca=(llvm::AllocaInst*)val;
            ty=getLLVMType(expr->getType());
            auto* gep=Builder.CreateInBoundsGEP(ty, alloca, 
            {llvm::ConstantInt::get(CTX, llvm::APInt(1, 0, false)),
            llvm::ConstantInt::get(CTX, llvm::APInt(1, 0, false)),}
            , "lgep");
            return gep;
        }
        else if(types::isUserDefined(expr->getType()))
        {
            return val;
        }

        return Builder.CreateLoad(ty, val, expr->getName());
    }
    llvm::Value* VCompiler::compileVariableDefinition(VariableDefAST* const def) // CHANGES REQUIRED: SRET ATTRIBUTE FOR ARRAYS
    {
        llvm::Value* lhs;
        llvm::MaybeAlign lhs_align;
        auto const& value=def->getValue();

        if(def->isReturned() && current_func_single_sret)
        {
            auto* arg=currentFunction->getArg(0);
            lhs=arg;
            lhs_align=arg->getParamAlign();
        }
        else
        {
            auto* alloca=namedValues[def->getName()];
            lhs=alloca;
            lhs_align=alloca->getAlign();
        }

        if(!value)
        {
            return lhs;
        }

        if(types::isUserDefined(def->getType())) // If it lhs is a struct
        {
            if(value->asttype==ast_call)
            {
                auto* func=analyzer->getFunction(((CallExprAST*)def->getValue())->getIName().name);
                llvm::CallInst* call;

                if(func->doesRequireSelfRef())
                    call=(llvm::CallInst*)compileCallExpr((CallExprAST*)def->getValue(), lhs);
                else
                    call=(llvm::CallInst*)compileExpr(def->getValue());

                if(!func->isConstructor())
                {
                    call=pushFrontToCallInst(lhs, call);
                    auto* ty=getLLVMType(func->getReturnType(), false);
                    uint64_t align=data_layout->getStructLayout((llvm::StructType*)ty)->getAlignment().value();
                    call->addParamAttr(0, llvm::Attribute::get(CTX, llvm::Attribute::StructRet, ty));
                    call->addParamAttr(0, llvm::Attribute::get(CTX, llvm::Attribute::Alignment, align));
                }

                return lhs;
            }
            else
            {
                auto* val=compileExpr(def->getValue());
                auto* alloca_rhs=(llvm::AllocaInst*)getValueAsAlloca(val);

                auto size=llvm::APInt(64, def->getType()->getSize(), false);
                auto* memcpy=Builder.CreateMemCpy(lhs, lhs_align, alloca_rhs, lhs_align, llvm::ConstantInt::get(CTX, size));
            }
        }
        else if(def->getType()->getType()==types::EType::Array)
        {
            if(value->asttype==ast_call)
            {
                auto* call=(llvm::CallInst*)compileExpr(value);
                pushFrontToCallInst(lhs, call);

                return lhs;
            }
            else
            {
                /* Creating a Memcpy call */

                // Compile the Array constant
                auto* val=compileExpr(value);

                // Set the size of the array
                auto* array_ast=(ArrayExprAST* const)value;
                std::size_t size=array_ast->getType()->getSize();

                // Create the memcpy call
                auto* call_inst=Builder.CreateMemCpy(lhs, lhs_align, val, lhs_align, llvm::ConstantInt::get(CTX, llvm::APInt(64, size, true)));
            }
        }
        else
        {   
            auto* val=compileExpr(value);
            Builder.CreateStore(val, lhs);
        }

        return lhs;
    }
    llvm::Value* VCompiler::compileVariableAssign(VariableAssignAST* const assign)
    {
        auto* lhs=compileExpr(assign->getLHS());
        auto* value=compileExpr(assign->getRHS());

        llvm::Value* ptr;

        if(assign->getLHS()->asttype==ast_array_access || assign->getLHS()->asttype==ast_type_access)
        {
            // If the lhs is an access, extract the pointer operand and delete the load

            auto* load_inst=llvm::dyn_cast<llvm::LoadInst>(lhs);
            ptr=load_inst->getPointerOperand();
            load_inst->eraseFromParent();
        }
        else if(assign->getLHS()->getType()->getType()==types::EType::Custom) // If lhs is a struct
        {
            auto* alloca_lhs=(llvm::AllocaInst*)getValueAsAlloca(lhs);
            auto* alloca_rhs=(llvm::AllocaInst*)getValueAsAlloca(value);

            auto lhs_align=alloca_lhs->getAlign();
            auto rhs_align=alloca_rhs->getAlign();

            auto size=llvm::APInt(64, assign->getLHS()->getType()->getSize(), false);
            auto* memcpy=Builder.CreateMemCpy(alloca_lhs, lhs_align, alloca_rhs, rhs_align, llvm::ConstantInt::get(CTX, size));

            return memcpy;
        }
        else
        {
            ptr=getValueAsAlloca(lhs);
        }

        if(assign->is_shorthand)
        {
            auto* sym=assign->getShorthandOperator();
            bool expr_is_fp=false;

            auto* lhs_type=assign->getLHS()->getType();
            auto* rhs_type=assign->getRHS()->getType();

            if(types::isTypeFloatingPoint(lhs_type) || types::isTypeFloatingPoint(rhs_type))
            {
                expr_is_fp=true;
            }
            
            auto* load=Builder.CreateLoad(getLLVMType(lhs_type), ptr);
            value=createBinaryOperation(load, value, sym, expr_is_fp);
        }

        return Builder.CreateStore(value, ptr);
    }
    llvm::Value* VCompiler::compileVariableArrayAccess(VariableArrayAccessAST* const access)
    {
        /* Updated to multi index access */
        llvm::Value* expr;
        auto* exp=compileExpr(access->getExpr());
        if(access->getExpr()->asttype==ast_var)
        {
            auto* var=currentFunctionAST->getVariable(((VariableExprAST*)access->getExpr())->getName());
            if(var->isReturned() && !var->isArgument())
            {
                expr=exp;
            }
            else if(var->isArgument())
            {
                bool offset=(current_func_ret_ty);
                expr=currentFunction->getArg(currentFunctionAST->getArgumentIndex(var->getName()) + offset);
            }
            else
                expr=getValueAsAlloca(exp);
        }
        else
            expr=getValueAsAlloca(exp);
        
        auto* ty=getLLVMType(access->getExpr()->getType());

        for(auto const& elem : access->getIndices())
        {
            auto* indx=compileExpr(elem.get());
            expr=Builder.CreateInBoundsGEP(ty, expr, {llvm::ConstantInt::get(CTX, llvm::APInt(64, 0, false)), indx}, "agep");
            if(ty->isArrayTy())
                ty=ty->getArrayElementType();
        }

        return Builder.CreateLoad(ty, expr);
    }
    llvm::Value* VCompiler::compileCastExpr(CastExprAST* const cast_expr)
    {
        if(cast_expr->isNonUserDefined())
        {
            auto* const dest_type=cast_expr->getDestType();
            auto* const src_type=cast_expr->getSourceType();

            bool is_dest_fp=types::isTypeFloatingPoint(dest_type);
            bool is_src_fp=types::isTypeFloatingPoint(src_type);

            auto* expr=compileExpr(cast_expr->getExpr());

            if(is_dest_fp xor is_src_fp)
            {
                if(is_dest_fp)
                {
                    if(src_type->is_signed) return Builder.CreateSIToFP(expr, getLLVMType(dest_type));
                    else                    return Builder.CreateUIToFP(expr, getLLVMType(dest_type));
                }
                else
                {
                    if(dest_type->is_signed) return Builder.CreateFPToSI(expr, getLLVMType(dest_type));
                    else                     return Builder.CreateFPToUI(expr, getLLVMType(dest_type));
                }
            }
            else if(is_dest_fp and is_src_fp)
            {
                if(dest_type->getSize() > src_type->getSize())
                {
                    auto* fpext=Builder.CreateFPExt(expr, getLLVMType(dest_type));
                    return fpext;
                }
                else
                {
                    auto* fptrunc=Builder.CreateFPTrunc(expr, getLLVMType(src_type));
                    return fptrunc;
                }
            }
            else
            {
                if(dest_type->getSize() > src_type->getSize())
                {
                    auto* zext=Builder.CreateZExt(expr, getLLVMType(dest_type));
                    return zext;
                }
                else
                {
                    auto* trunc=Builder.CreateTrunc(expr, getLLVMType(dest_type));
                    return trunc;
                }
            }
        }
        else
        {
            return nullptr;
        }
    }

    llvm::Value* VCompiler::compileBinopExpr(BinaryExprAST* const expr)
    {
        auto* lhs=compileExpr(expr->getLHS());

        if(expr->getOp()->type!=tok_and && expr->getOp()->type!=tok_or)
        {   
            auto* rhs=compileExpr(expr->getRHS());
            bool expr_is_fp=types::isTypeFloatingPoint(expr->getType());
            return createBinaryOperation(lhs, rhs, expr->getOp(), expr_is_fp);
        }
        
        // Operator is either `and` or `or`
        bool type_is_and=(expr->getOp()->type==tok_and);
        const char* bb_rhs_name="",* bb_end_name="";
        if(type_is_and) { bb_rhs_name="and.op.rhs"; bb_end_name="and.op.end"; }
        else            { bb_rhs_name="or.op.rhs";  bb_end_name="or.op.end"; }

        auto* entry_bb=Builder.GetInsertBlock();
        auto* bb_rhs=llvm::BasicBlock::Create(CTX, bb_rhs_name, currentFunction);
        auto* bb_end=llvm::BasicBlock::Create(CTX, bb_end_name, currentFunction);

        if(type_is_and)
            Builder.CreateCondBr(lhs, bb_rhs, bb_end);
        else
            Builder.CreateCondBr(lhs, bb_end, bb_rhs);
        
        Builder.SetInsertPoint(bb_rhs);
        auto* rhs=compileExpr(expr->getRHS());
        Builder.CreateBr(bb_end);

        Builder.SetInsertPoint(bb_end);
        auto* phi=Builder.CreatePHI(llvm::Type::getInt1Ty(CTX), 2, "op.res");

        phi->addIncoming(llvm::ConstantInt::get(CTX, llvm::APInt(1, !type_is_and)), entry_bb);
        phi->addIncoming(rhs, bb_rhs);

        return phi;
    }

    std::vector<llvm::Value*> VCompiler::compileBlock(std::vector<std::unique_ptr<ExprAST>> const& block)
    {
        std::vector<llvm::Value*> values;
        for(const auto& expr : block)
        {
            // This happens if the function returns a struct or a list
            if(expr->asttype==ast_return && current_func_single_sret)
            {
                auto* ret=(ReturnExprAST*)expr.get();

                if(ret->getValue()->asttype==ast_var)
                {
                    auto* var=currentFunctionAST->getVariable(((VariableDefAST*)ret->getValue())->getName());
                    if(var->isArgument())
                    {
                        createSRetMemCpyForArg(ret);
                        break;
                    }
                    else
                    {
                        Builder.CreateBr(currentFunctionEndBB);
                        break;
                    }
                }
            }
            else if(expr->asttype==ast_vardef)
            {
                auto* var=(VariableDefAST*)expr.get();
                
                if(var->isArgument())
                    continue;
            }

            values.push_back(compileExpr(expr.get()));
        }

        return values;
    }

    llvm::Value* VCompiler::compileIfThen(IfThenExpr* const ifthen)
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
    llvm::Value* VCompiler::compileIfElse(IfExprAST* const ifelse)
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

    llvm::Value* VCompiler::compileForExpr(ForExprAST* const forexpr)
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
        auto* br=Builder.CreateCondBr(cond, forloop, forcont);

        Builder.SetInsertPoint(forloop);
        compileBlock(forexpr->getBody());
        auto* incr=compileExpr(forexpr->getIncr());
        createBrIfNoTerminator(forbool);

        Builder.SetInsertPoint(forcont);
        return br;
    }
    llvm::Value* VCompiler::compileWhileExpr(WhileExprAST* const whileexpr)
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
    llvm::Value* VCompiler::compileBreakExpr(BreakExprAST* const breakexpr)
    {
        compileExpr(breakexpr->getAfterBreak());

        return Builder.CreateBr(currentLoopEndBB);
    }
    llvm::Value* VCompiler::compileContinueExpr(ContinueExprAST* const continueexpr)
    {
        compileExpr(continueexpr->getAfterCont());

        return Builder.CreateBr(currentLoopBodyBB);
    }

    llvm::Value* VCompiler::compileCallExpr(CallExprAST* const expr, llvm::Value* parent_struct)
    {
        std::string func_name;
        auto* afunc=analyzer->getFunction(expr->getIName().name);

        if(afunc->is_extern())
        {
            func_name=afunc->getIName().name;
        }
        else
        {
            func_name=afunc->getName();
        }

        auto* func=Module->getFunction(func_name);

        std::vector<llvm::Value*> args;
        if(afunc->doesRequireSelfRef())
        {
            args.push_back(parent_struct);
        }
        for(auto& arg : expr->getArgs())
        {
            auto* carg=compileExpr(arg.get());
            args.push_back(carg);
        }

        llvm::CallInst* call;
        if(func->getReturnType()->isVoidTy())
        {
            call=Builder.CreateCall(func, args);
        }
        else
        {
            call=Builder.CreateCall(func, args, "calltmp");
        }

        unsigned int indx=afunc->doesRequireSelfRef();
        if(afunc->doesRequireSelfRef())
        {
            call->addParamAttr(0, llvm::Attribute::NoUndef);
            call->addParamAttr(0, llvm::Attribute::NonNull);
            auto* ty=(llvm::StructType*)getLLVMType(afunc->getReturnType(), false);
            uint64_t align=data_layout->getStructLayout(ty)->getAlignment().value();
            call->addParamAttr(0, llvm::Attribute::get(CTX, llvm::Attribute::Alignment, align));
            call->addParamAttr(0, llvm::Attribute::get(CTX, llvm::Attribute::Dereferenceable, afunc->getReturnType()->getSize()));
        }
        for(auto& arg : expr->getArgs())
        {
            call->addParamAttr(indx, llvm::Attribute::NoUndef);
            if(types::isUserDefined(arg->getType()))
            {
                auto* ty=getLLVMType(arg->getType());
                uint64_t align=data_layout->getStructLayout((llvm::StructType*)ty)->getAlignment().value();
                call->addParamAttr(indx, llvm::Attribute::get(CTX, llvm::Attribute::ByVal, ty));
                call->addParamAttr(indx, llvm::Attribute::get(CTX, llvm::Attribute::Alignment, align));
            }

            indx++;
        }

        return call;
    }
    llvm::Value* VCompiler::compileReturnExpr(ReturnExprAST* const expr)
    {
        auto* expr_val=compileExpr(expr->getValue());

        if(types::isUserDefined(currentFunctionAST->getReturnType()) 
        || currentFunctionAST->getReturnType()->getType()==types::EType::Array)
        {
            llvm::Value* arg0=currentFunction->getArg(0);
            
            std::size_t nsize;
            auto align=((llvm::Argument*)arg0)->getParamAlign();
            if(types::isUserDefined(currentFunctionAST->getReturnType()))
            {
                // If its a struct
                nsize=types::custom_type_sizes[((types::Custom*)expr->getValue()->getType())->getName()];
            }
            else
            {
                // If its an array
                nsize=((types::Array*)expr->getValue()->getType())->getSize();

                if(auto* gep=llvm::dyn_cast<llvm::GetElementPtrInst>(expr_val))
                {
                    expr_val=gep->getPointerOperand();
                    gep->eraseFromParent();
                }
            }
            
            auto* size=llvm::ConstantInt::get(CTX, llvm::APInt(64, nsize, false));
            auto* memcpy=Builder.CreateMemCpy(arg0, align, expr_val, align, size);
            Builder.CreateBr(currentFunctionEndBB);

            return memcpy;
        }
        auto* value=Builder.CreateStore(expr_val, namedValues["retval"]);
        Builder.CreateBr(currentFunctionEndBB);

        return value;
    }
    llvm::Function* VCompiler::compilePrototype(PrototypeAST* const proto)
    {
        auto const& proto_args=proto->getArgs();
        std::vector<llvm::Type*> args(proto_args.size());

        llvm::Type* func_ret_type;
        bool func_rets_ty=false;

        if((types::isUserDefined(proto->getReturnType()) || proto->getReturnType()->getType()==types::EType::Array) && !proto->isConstructor())
        {
            auto* ty=getLLVMType(proto->getReturnType());
            func_ret_type=llvm::Type::getVoidTy(CTX);

            args.resize(args.size()+1);
            args[0]=llvm::PointerType::get(ty, 0);
            func_rets_ty=true;
        }
        else
        {
            if(proto->isConstructor()) func_ret_type=llvm::Type::getVoidTy(CTX);
            else func_ret_type=getLLVMType(proto->getReturnType());
        }

        for(size_t i=0; i<proto_args.size(); i++)
        {
            auto* llvm_type=getLLVMType(proto_args[i]->getType());

            if(llvm_type->isArrayTy() || llvm_type->isStructTy())
            {
                llvm_type=llvm::PointerType::get(llvm_type, 0);
            }

            args[i+func_rets_ty]=llvm_type;
        }

        llvm::FunctionType* func_type=llvm::FunctionType::get(func_ret_type, args, false);
        llvm::Function* func=llvm::Function::Create(func_type, llvm::Function::ExternalLinkage, proto->getName(), Module.get());

        // if function returns struct, then normal indexes start from 1, otherwise 0
        if(proto->doesRequireSelfRef())
        {
            auto* arg=func->getArg(0);
            auto* ty=(llvm::StructType*)getLLVMType(proto->getReturnType(), false);

            llvm::AttrBuilder attrs(CTX);
            attrs.addAttribute(llvm::Attribute::NoUndef);
            attrs.addAttribute(llvm::Attribute::NonNull);
            attrs.addAlignmentAttr(data_layout->getStructLayout(ty)->getAlignment());
            attrs.addDereferenceableAttr(proto->getReturnType()->getSize());
            arg->addAttrs(attrs);
            arg->setName("self");
        }
        for(unsigned idx=proto->doesRequireSelfRef(); idx<proto_args.size(); ++idx)
        {
            auto* arg=func->getArg(idx+func_rets_ty);
            arg->setName("a"+proto_args[idx]->getName());

            if(types::isUserDefined(proto_args[idx]->getType()))
            {
                auto* ty=getLLVMType(proto_args[idx]->getType(), false);

                llvm::AttrBuilder attrs(CTX);
                attrs.addAttribute(llvm::Attribute::NoUndef);
                attrs.addByValAttr(ty);
                attrs.addAlignmentAttr(data_layout->getStructLayout((llvm::StructType*)ty)->getAlignment());
                arg->addAttrs(attrs);
            }
        }

        if(func_rets_ty)
        {
            auto* ty=getLLVMType(proto->getReturnType(), false);
            llvm::AttrBuilder attrs(CTX);
            attrs.addStructRetAttr(ty);
            attrs.addAttribute(llvm::Attribute::NoAlias);
            attrs.addAlignmentAttr(data_layout->getStructLayout((llvm::StructType*)ty)->getAlignment());

            func->addParamAttrs(0, attrs);
        }

        func->addFnAttr(llvm::Attribute::get(CTX, "wasm-export-name", func->getName()));
        func->setVisibility(llvm::GlobalValue::DefaultVisibility);

        return func;
    }
    llvm::Function* VCompiler::compileExtern(std::string const& name)
    {
        auto* ext=(ExternAST*)analyzer->getFunction(name);
        llvm::Function* func=compilePrototype(ext->getProto());
        func->setName(ext->getIName().name);

        // Remove in release
        // func->print(error_os);
        return func;
    }
    llvm::Function* VCompiler::compileFunction(FunctionAST* const func)
    {
        llvm::Function* function=Module->getFunction(func->getName());
        auto func_ty=func->getReturnType()->getType();

        if(!function)
            function=compilePrototype(func->getProto());

        llvm::BasicBlock* bb=llvm::BasicBlock::Create(CTX, "entry", function);
        currentFunctionEndBB=llvm::BasicBlock::Create(CTX, "end", function);
        Builder.SetInsertPoint(bb);

        auto& func_args=func->getArgs();

        namedValues.clear();
        bool func_ret_ty=((func_ty==types::EType::Custom || func_ty==types::EType::Array) && !func->isConstructor());

        // Create return value
        llvm::Type* ret_type=getLLVMType(func->getReturnType());
        bool func_returns=(func_ty!=types::EType::Void && !func_ret_ty && !func->isConstructor());
        if(func_returns)
        {
            auto* ret_val=Builder.CreateAlloca(ret_type, nullptr, "retval");
            namedValues["retval"]=ret_val;
        }

        // Compile the block
        currentFunction=function;
        currentFunctionAST=func;

        // Check if only a single variable is returned
        bool single_var_ret=true;
        if(func->getLocals().size()>0)
        {
            auto it=func->getLocals().begin();
            auto first_var_name=it->first;
            std::advance(it, 1);
            while(it!=func->getLocals().end())
            {
                if(it->first != first_var_name)
                {
                    single_var_ret=false;
                    break;
                }
                ++it;
            }
        }
        
        current_func_single_sret=((single_var_ret && func_ret_ty) || func->isConstructor());
        current_func_ret_ty=func_ret_ty;

        for(auto& [vname, var]: func->getLocals())
        {
            if(var->isArgument())
                continue;
            if(var->isReturned() && current_func_single_sret)
                continue;
            
            createAllocaForVar(var);
        }

        compileBlock(func->getBody());
        createBrIfNoTerminator(currentFunctionEndBB);

        // Create the return instruction
        Builder.SetInsertPoint(currentFunctionEndBB);

        if(func_returns)
        {
            Builder.CreateRet(Builder.CreateLoad(ret_type, namedValues["retval"], "ret"));
        }
        else
        {
            Builder.CreateRetVoid();
        }

        if(func->getIName().name=="main")
        {
            function->removeFnAttr("wasm-export-name");
            function->setName("entry_main");
        }
        
        return function;
    }
    llvm::Function* VCompiler::compileFunction(std::string const& name)
    {
        return compileFunction((FunctionAST*)analyzer->getFunction(name));
    }

    llvm::StructType* VCompiler::compileStruct(std::string const& name, StructExprAST* struct_)
    {
        StructExprAST* st;

        if(struct_)
        {
            st=struct_;
        }
        else
        {
            st=analyzer->getStruct(name);
        }

        std::vector<llvm::Type*> elements;

        auto const& vec=st->getMembersValues();
        std::vector<ExprAST*>::const_reverse_iterator it;
        for(it=vec.rbegin(); it!=vec.rend(); ++it)
        {
            auto const& expr=*it;
            if(expr->asttype==ast_struct)
            {
                auto* st=compileStruct("", ((StructExprAST*)expr));
                elements.push_back(st);
            }
            else
            {
                elements.push_back(getLLVMType(expr->getType()));
            }
        }

        auto struct_type=llvm::StructType::create(CTX, elements);
        auto struct_name="struct."+st->getName();
        struct_type->setName(struct_name);

        definedStructs[st->getName()]=struct_type;

        // Create the constructor
        compileFunction(st->getConstructor());

        return struct_type;
    }
    llvm::Value* VCompiler::compileTypeAccess(TypeAccessAST* const expr)
    {
        llvm::Value* sgep=nullptr;
        StructExprAST* st=nullptr;
        ExprAST* current_expr=expr;
        bool first_iter_completed=false;

        while(current_expr->asttype==ast_type_access)
        {
            // Load the type access ast and the pre-compiled struct type
            auto* current=(TypeAccessAST*)current_expr;
            auto* st_type=(types::Custom*)current->getParent()->getType();
            auto* st_ltype=definedStructs[st_type->getName()];

            // Compile the pointer
            llvm::Value* val;
            if(first_iter_completed)
            {
                val=sgep;
            }
            else
            {
                auto* exp=compileExpr(current->getParent());
                if(current->getParent()->asttype==ast_var)
                {
                    auto* var=currentFunctionAST->getVariable(((VariableExprAST*)current->getParent())->getName());
                    if(var->isReturned() && !var->isArgument())
                    {
                        val=exp;
                    }
                    else if(var->isArgument())
                    {
                        bool offset=(current_func_ret_ty);
                        val=currentFunction->getArg(currentFunctionAST->getArgumentIndex(var->getName()) + offset);
                    }
                    else
                        val=getValueAsAlloca(exp);
                }
                else
                    val=getValueAsAlloca(exp);
            }

            if(first_iter_completed)
            {
                st=(StructExprAST*)st->getMember(st_type->getName());
            }
            else
            {
                st=analyzer->getStruct(st_type->getName());
            }

            if(current->getChild()->asttype==ast_call)
            {
                return compileCallExpr((CallExprAST*)current->getChild(), val);
            }

            int indx=st->getMemberIndex(current->getIName());

            sgep=Builder.CreateStructGEP(st_ltype, val, indx, "sgep");
            current_expr=current->getChild();
            first_iter_completed=true;
        }

        auto* ty=getLLVMType(expr->getType());
        return Builder.CreateLoad(ty, sgep);
    }

    void VCompiler::resetModule()
    {
        Module=std::make_unique<llvm::Module>(Module->getName(), CTX);
    }
    void VCompiler::compileModule()
    {
        auto* mod=analyzer->getSourceModule();

        for(auto const& s:mod->getUnionStructs())
        {
            if(s->asttype==ast_struct)
            {
                std::string name=((StructExprAST*)s.get())->getName();
                compileStruct(name);
            }
            else
            {
                std::string name=((UnionExprAST*)s.get())->getName();
                //compileUnion(name);
            }
        }

        for(auto const& f:mod->getFunctions())
        {
            if(f->is_proto())
            {
                auto* proto=(PrototypeAST*)f.get();
                compilePrototype(proto);
            }
            else if(f->is_extern())
            {
                compileExtern(f->getIName().name);
            }
            else
            {
                auto* func=(FunctionAST*)f.get();
                compileFunction(func);
            }
        }

        current_func_single_sret=current_func_ret_ty=false;
        llvm::FunctionType* main_type=llvm::FunctionType::get(llvm::Type::getInt32Ty(CTX), false);
        llvm::Function* main_func=llvm::Function::Create(main_type, llvm::GlobalValue::ExternalLinkage, "main", Module.get());
        llvm::BasicBlock* bb=llvm::BasicBlock::Create(CTX, "entry", main_func);
        currentFunctionEndBB=llvm::BasicBlock::Create(CTX, "end", main_func);
        Builder.SetInsertPoint(bb);
        currentFunction=main_func;
        main_func->addFnAttr(llvm::Attribute::get(CTX, "wasm-export-name", "_main"));
        main_func->setVisibility(llvm::GlobalValue::DefaultVisibility);

        /* Create a temp main function */
        auto name=std::make_unique<VToken>("main", tok_id);
        std::vector<std::unique_ptr<VariableDefAST>> args;
        std::vector<std::unique_ptr<ExprAST>> stms;

        auto main_func_ast=std::make_unique<FunctionAST>(std::make_unique<PrototypeAST>(std::move(name), std::move(args), types::construct("int")), std::move(stms));
        currentFunctionAST=main_func_ast.get();
        
        for(auto const& var: mod->getPreExecutionStatementsVariables())
        {
            createAllocaForVar(var);
            main_func_ast->addVariable(var);
        }
        for(auto const& e: mod->getPreExecutionStatements())
        {
            compileExpr(e.get());
        }

        if(auto* pre_main_func=Module->getFunction("entry_main"))
        {
            if(!pre_main_func->getReturnType()->isVoidTy())
            {
                auto* retval=Builder.CreateCall(pre_main_func, std::nullopt, "retval");
                Builder.CreateBr(currentFunctionEndBB);
                Builder.SetInsertPoint(currentFunctionEndBB);
                Builder.CreateRet(retval);
            }
            else
            {
                Builder.CreateCall(pre_main_func);
                Builder.CreateBr(currentFunctionEndBB);
                Builder.SetInsertPoint(currentFunctionEndBB);
                Builder.CreateRet(llvm::ConstantInt::get(CTX, llvm::APInt(32, 0, false)));
            }
        }
        else
        {
            Builder.CreateBr(currentFunctionEndBB);
            Builder.SetInsertPoint(currentFunctionEndBB);
            Builder.CreateRet(llvm::ConstantInt::get(CTX, llvm::APInt(32, 0, false)));
        }
    }

    llvm::Module* const VCompiler::getModule() const
    {
        return Module.get();
    }
    std::string const& VCompiler::getCompiledOutput()
    {
        output_ir="";
        llvm::raw_string_ostream os(output_ir);
        Module->print(os, nullptr);
        return output_ir;
    }
    VAnalyzer* const VCompiler::getAnalyzer()  const
    {
        return analyzer.get();
    }

    void VCompiler::runOptimizationPasses(llvm::TargetMachine* tm, Optimization opt_level, bool enable_lto)
    {
        #ifndef VIRE_NO_PASSES
        
        llvm::LoopAnalysisManager lam;
        llvm::FunctionAnalysisManager fam;
        llvm::CGSCCAnalysisManager cam;
        llvm::ModuleAnalysisManager mam;

        llvm::PipelineTuningOptions pio;
        pio.LoopInterleaving=true;
        pio.LoopUnrolling=true;
        pio.LoopVectorization=true;
        pio.SLPVectorization=true;
        pio.MergeFunctions=true;
        llvm::PassBuilder pass_builder(tm, pio);

        fam.registerPass([&] { return pass_builder.buildDefaultAAPipeline(); });
        pass_builder.registerModuleAnalyses(mam);
        pass_builder.registerCGSCCAnalyses(cam);
        pass_builder.registerFunctionAnalyses(fam);
        pass_builder.registerLoopAnalyses(lam);
        pass_builder.crossRegisterProxies(lam, fam, cam, mam);

        pass_builder.registerOptimizerLastEPCallback([&] (llvm::ModulePassManager& mpm, llvm::OptimizationLevel)
        {
            mpm.addPass(llvm::createModuleToFunctionPassAdaptor(llvm::DSEPass{}));
        });

        llvm::ModulePassManager passmgr;
        llvm::OptimizationLevel lvl;

        if(opt_level == Optimization::O0)
        {
            passmgr=pass_builder.buildO0DefaultPipeline(llvm::OptimizationLevel::O0, enable_lto);
        }
        else
        {
            switch(opt_level)
            {
                case Optimization::O1: lvl=llvm::OptimizationLevel::O1; break;
                case Optimization::O2: lvl=llvm::OptimizationLevel::O2; break;
                case Optimization::O3: lvl=llvm::OptimizationLevel::O3; break;
                case Optimization::Os: lvl=llvm::OptimizationLevel::Os; break;
                case Optimization::Oz: lvl=llvm::OptimizationLevel::Oz; break;

                default: lvl=llvm::OptimizationLevel::O0;
            }

            if(enable_lto)
            {
                passmgr=pass_builder.buildLTOPreLinkDefaultPipeline(lvl);
            }
            else
            {
                passmgr=pass_builder.buildPerModuleDefaultPipeline(lvl);
            }
        }

        passmgr.run(*Module, mam);

        #endif
    }
    llvm::TargetMachine* VCompiler::compileInternal(std::string const& target_str)
    {
        std::string target_triple;
        if(target_str=="sys" || target_str=="")
        {
            target_triple=llvm::sys::getDefaultTargetTriple();
        }
        else
        {
            target_triple=target_str;
        }
    
    #ifdef VIRE_ENABLE_ONLY
        SPECIFIC_INIT_TARGET_INFO(VIRE_ENABLE_ONLY);
        SPECIFIC_INIT_TARGET(VIRE_ENABLE_ONLY);
        SPECIFIC_INIT_TARGET_MC(VIRE_ENABLE_ONLY);
        SPECIFIC_INIT_ASM_PARSER(VIRE_ENABLE_ONLY);
        SPECIFIC_INIT_ASM_PRINTER(VIRE_ENABLE_ONLY);
    #endif
    #ifndef VIRE_ENABLE_ONLY
        llvm::InitializeAllTargetInfos();
        llvm::InitializeAllTargets();
        llvm::InitializeAllTargetMCs();
        llvm::InitializeAllAsmParsers();
        llvm::InitializeAllAsmPrinters();
    #endif
        
        std::string error;
        auto* target=llvm::TargetRegistry::lookupTarget(target_triple, error);

        if(!target)
        {
            llvm::errs() << "Target not found:\n" << error;
            return nullptr;
        }

        std::string cpu="generic";
        std::string features;

        // POSSIBLY DANGEROUS, TO BE CHANGED
    #ifndef VIRE_ENABLE_ONLY
        cpu=llvm::sys::getHostCPUName();
    #endif

        llvm::TargetOptions opt;
        auto rm=llvm::Optional<llvm::Reloc::Model>();

        auto* target_machine=target->createTargetMachine(target_triple, cpu, features, opt, rm);

        Module->setDataLayout(target_machine->createDataLayout());
        Module->setTargetTriple(target_triple);

        return target_machine;
    }
    std::vector<unsigned char> VCompiler::compileToString(std::string const& target_str, Optimization opt_level, bool enable_lto)
    {
        llvm::SmallString<1> out;
        llvm::raw_svector_ostream os(out);

        auto* target_machine=compileInternal(target_str);

        if(!target_machine)
        {
            return std::vector<unsigned char>();
        }

        runOptimizationPasses(target_machine, opt_level, enable_lto);

        llvm::legacy::PassManager legacy_passmgr;
        target_machine->addPassesToEmitFile(legacy_passmgr, os, nullptr, file_type);
        legacy_passmgr.run(*Module);

        auto bytestr=out.str().str();
        std::vector<unsigned char> ret(bytestr.begin(), bytestr.end());

        delete target_machine;

        return ret;
    }
    void VCompiler::compileToFile(std::string const& filename, std::string const& target_str, Optimization opt_level, bool enable_lto)
    {
        std::error_code ec;
        llvm::raw_fd_ostream os(filename, ec, llvm::sys::fs::OF_None);
        
        auto* target_machine=compileInternal(target_str);

        if(!target_machine)
        {
            return;
        }

        runOptimizationPasses(target_machine, opt_level, enable_lto);

        llvm::legacy::PassManager legacy_passmgr;
        target_machine->addPassesToEmitFile(legacy_passmgr, os, nullptr, file_type);
        legacy_passmgr.run(*Module);
        os.flush();

        delete target_machine;
    }
}
