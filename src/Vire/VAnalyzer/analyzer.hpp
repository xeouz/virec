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
    const std::unique_ptr<errors::ErrorBuilder>& builder;

    // Source Code
    std::string code;
public:
    VAnalyzer(const std::unique_ptr<errors::ErrorBuilder>& builder=nullptr, const std::string& code="")
    : builder(builder), code(code) {}

    bool isVarDefined(const std::string& name);
    bool isStructDefined(const std::string& name);
    bool isUnionDefined(const std::string& name);
    bool isClassDefined(const std::string& name);
    bool isFuncDefined(const std::string& name);
    bool isFuncDefined(const std::unique_ptr<FunctionBaseAST>& func);
    
    bool addVar(std::unique_ptr<VariableDefAST> var);
    bool addFunc(std::unique_ptr<FunctionBaseAST> func);
    bool addStruct(std::unique_ptr<StructExprAST> struct_);
    bool addUnion(std::unique_ptr<UnionExprAST> union_);
    bool addClass(std::unique_ptr<ClassAST> class_);
    const std::unique_ptr<VariableDefAST>& getVar(const std::string& name);

    std::string getType(const std::unique_ptr<ExprAST>& expr);
    std::string getType(const std::unique_ptr<VariableExprAST>& var);
    std::string getType(const std::unique_ptr<TypedVarAST>& var);
    std::string getType(const std::unique_ptr<ArrayExprAST>& arr);

    // Verification functions
    // Return Empty "" String if valid

    bool verifyVar(std::unique_ptr<VariableExprAST> var);
    bool verifyVarDef(std::unique_ptr<VariableDefAST> var);
    bool verifyTypedVar(std::unique_ptr<TypedVarAST> var);
    bool verifyVarAssign(std::unique_ptr<VariableAssignAST> var);

    // Constant verification due to overflows and invalid escape sequences
    bool verifyInt(std::unique_ptr<IntExprAST> int_); 
    bool verifyFloat(std::unique_ptr<FloatExprAST> float_);
    bool verifyDouble(std::unique_ptr<DoubleExprAST> double_);
    bool verifyChar(std::unique_ptr<CharExprAST> char_);
    bool verifyStr(std::unique_ptr<StrExprAST> str);
    bool verifyArray(std::unique_ptr<ArrayExprAST> array);
    
    bool verifyFor(std::unique_ptr<ForExprAST> for_);
    bool verifyWhile(std::unique_ptr<WhileExprAST> while_);
    bool verifyBreak(std::unique_ptr<BreakExprAST> break_);
    bool verifyContinue(std::unique_ptr<ContinueExprAST> continue_);

    bool verifyCall(std::unique_ptr<CallExprAST> call);
    bool verifyProto(std::unique_ptr<PrototypeAST> proto);
    bool verifyExtern(std::unique_ptr<ExternAST> extern_);
    bool verifyFunction(std::unique_ptr<FunctionAST> function);
    bool verifyReturn(std::unique_ptr<ReturnExprAST> return_);

    bool verifyUnop(std::unique_ptr<UnaryExprAST> unop);
    bool verifyBinop(std::unique_ptr<BinaryExprAST> binop);

    bool verifyClass(std::unique_ptr<ClassAST> class_);
    bool verifyNew(std::unique_ptr<NewExprAST> new_);
    bool verifyDelete(std::unique_ptr<DeleteExprAST> delete_);

    bool verifyUnion(std::unique_ptr<UnionExprAST> union_);
    bool verifyStruct(std::unique_ptr<StructExprAST> struct_);

    bool verifyIf(std::unique_ptr<IfExprAST> if_);

    bool verifyUnsafe(std::unique_ptr<UnsafeAST> unsafe);
    bool verifyReference(std::unique_ptr<ReferenceExprAST> reference);

    bool verifyCode(std::unique_ptr<CodeAST> code);

    bool verifyExpr(std::unique_ptr<ExprAST> expr);

    std::vector<std::string> getErrors();
};

}