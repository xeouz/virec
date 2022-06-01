#pragma once

#include "../AST-Parse/Include.hpp"
#include "../AST-Defs/Include.hpp"

namespace vire
{

class VAnalyzer
{
    std::unique_ptr<Virelex> lexer;
public:
    VAnalyzer(std::unique_ptr<Vireparse> parser);
    ~VAnalyzer();

    unsigned char verifyVar(std::unique_ptr<VariableExprAST> var);
    unsigned char verifyVarDef(std::unique_ptr<VariableDefAST> var);
    unsigned char verifyTypedVar(std::unique_ptr<TypedVarAST> var);
    unsigned char verifyVarAssign(std::unique_ptr<VariableAssignAST> var);

    unsigned char verifyFor(std::unique_ptr<ForExprAST> for_);
    unsigned char verifyWhile(std::unique_ptr<WhileExprAST> while_);
    unsigned char verifyBreak(std::unique_ptr<BreakExprAST> break_);
    unsigned char verifyContinue(std::unique_ptr<ContinueExprAST> continue_);

    unsigned char verifyInt(std::unique_ptr<IntExprAST> int_);
    unsigned char verifyFloat(std::unique_ptr<FloatExprAST> float_);
    unsigned char verifyDouble(std::unique_ptr<DoubleExprAST> double_);
    unsigned char verifyChar(std::unique_ptr<CharExprAST> char_);
    unsigned char verifyStr(std::unique_ptr<StrExprAST> str);
    unsigned char verifyArray(std::unique_ptr<ArrayExprAST> array);

    unsigned char verifyCall(std::unique_ptr<CallExprAST> call);
    unsigned char verifyProto(std::unique_ptr<PrototypeAST> proto);
    unsigned char verifyExtern(std::unique_ptr<ExternAST> extern_);
    unsigned char verifyFunction(std::unique_ptr<FunctionAST> function);
    unsigned char verifyReturn(std::unique_ptr<ReturnExprAST> return_);

    unsigned char verifyUnop(std::unique_ptr<UnaryExprAST> unop);
    unsigned char verifyBinop(std::unique_ptr<BinaryExprAST> binop);

    unsigned char verifyClass(std::unique_ptr<ClassAST> class_);
    unsigned char verifyNew(std::unique_ptr<NewExprAST> new_);
    unsigned char verifyDelete(std::unique_ptr<DeleteExprAST> delete_);

    unsigned char verifyUnion(std::unique_ptr<UnionExprAST> union_);
    unsigned char verifyStruct(std::unique_ptr<StructExprAST> struct_);

    unsigned char verifyIf(std::unique_ptr<IfExprAST> if_);

    unsigned char verifyUnsafe(std::unique_ptr<UnsafeAST> unsafe);
    unsigned char verifyReference(std::unique_ptr<ReferenceExprAST> reference);
};

}