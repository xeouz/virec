#pragma once

#include "../AST-Parse/Include.hpp"
#include "../AST-Defs/Include.hpp"

#include "../Error-Builder/Include.hpp"

namespace vire
{

class VAnalyzer
{
    // Symbol Tables
    std::vector<std::unique_ptr<VariableDefAST>> variables;
    std::vector<std::unique_ptr<FunctionBaseAST>> functions;
    std::vector<std::unique_ptr<StructExprAST>> structs;
    std::vector<std::unique_ptr<UnionExprAST>> unions;
    std::vector<std::unique_ptr<ClassAST>> classes;

    // Error Builder
    std::unique_ptr<errors::ErrorBuilder> errorBuilder;
public:
    VAnalyzer() {}
    VAnalyzer(std::unique_ptr<errors::ErrorBuilder> builder)
    : errorBuilder(std::move(builder)) {}

    bool isVarDefined(const std::string& name);
    bool isStructDefined(const std::string& name);
    bool isUnionDefined(const std::string& name);
    bool isClassDefined(const std::string& name);
    bool isFuncDefined(const std::string& name);
    bool isFuncDefined(const std::unique_ptr<FunctionBaseAST>& func);
    
    bool addVar(const std::unique_ptr<ExprAST>& var);
    bool addFunc(const std::unique_ptr<ExprAST>& func);
    bool addStruct(const std::unique_ptr<ExprAST>& struct_);
    bool addUnion(const std::unique_ptr<ExprAST>& union_);
    bool addClass(const std::unique_ptr<ExprAST>& class_);

    // Verification functions
    // Return Empty "" String if valid, else return error message

    std::string verifyVar(const std::unique_ptr<VariableExprAST>& var);
    std::string verifyVarDef(const std::unique_ptr<VariableDefAST>& var);
    std::string verifyTypedVar(const std::unique_ptr<TypedVarAST>& var);
    std::string verifyVarAssign(const std::unique_ptr<VariableAssignAST>& var);

    std::string verifyFor(const std::unique_ptr<ForExprAST>& for_);
    std::string verifyWhile(const std::unique_ptr<WhileExprAST>& while_);
    std::string verifyBreak(const std::unique_ptr<BreakExprAST>& break_);
    std::string verifyContinue(const std::unique_ptr<ContinueExprAST>& continue_);

    std::string verifyInt(const std::unique_ptr<IntExprAST>& int_);
    std::string verifyFloat(const std::unique_ptr<FloatExprAST>& float_);
    std::string verifyDouble(const std::unique_ptr<DoubleExprAST>& double_);
    std::string verifyChar(const std::unique_ptr<CharExprAST>& char_);
    std::string verifyStr(const std::unique_ptr<StrExprAST>& str);
    std::string verifyArray(const std::unique_ptr<ArrayExprAST> array);

    std::string verifyCall(const std::unique_ptr<CallExprAST>& call);
    std::string verifyProto(const std::unique_ptr<PrototypeAST>& proto);
    std::string verifyExtern(const std::unique_ptr<ExternAST>& extern_);
    std::string verifyFunction(const std::unique_ptr<FunctionAST>& function);
    std::string verifyReturn(const std::unique_ptr<ReturnExprAST>& return_);

    std::string verifyUnop(const std::unique_ptr<UnaryExprAST>& unop);
    std::string verifyBinop(const std::unique_ptr<BinaryExprAST>& binop);

    std::string verifyClass(const std::unique_ptr<ClassAST>& class_);
    std::string verifyNew(const std::unique_ptr<NewExprAST>& new_);
    std::string verifyDelete(const std::unique_ptr<DeleteExprAST>& delete_);

    std::string verifyUnion(const std::unique_ptr<UnionExprAST>& union_);
    std::string verifyStruct(const std::unique_ptr<StructExprAST>& struct_);

    std::string verifyIf(const std::unique_ptr<IfExprAST>& if_);

    std::string verifyUnsafe(const std::unique_ptr<UnsafeAST>& unsafe);
    std::string verifyReference(const std::unique_ptr<ReferenceExprAST>& reference);

    std::string verifyCode(const std::unique_ptr<CodeAST>& code);

    std::vector<std::string> getErrors();
};

}