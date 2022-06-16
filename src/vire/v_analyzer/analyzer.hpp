#pragma once

#include "../includes.hpp"
#include __VIRE_PARSE_PATH
#include __VIRE_AST_PATH
#include __VIRE_ERRORS_PATH

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
    bool isStdFunc(const std::string& name);
    unsigned int getFuncArgCount(const std::string& name);
    
    bool addVar(std::unique_ptr<VariableDefAST> var);
    bool addProto(std::unique_ptr<PrototypeAST> proto);
    bool addExtern(std::unique_ptr<ExternAST> extern_);
    bool addFunc(std::unique_ptr<FunctionAST> func);
    bool addStruct(std::unique_ptr<StructExprAST> struct_);
    bool addUnion(std::unique_ptr<UnionExprAST> union_);
    bool addClass(std::unique_ptr<ClassAST> class_);
    const std::unique_ptr<VariableDefAST>& getVar(const std::string& name);

    std::string getType(const std::unique_ptr<ExprAST>& expr);
    std::string getType(const std::unique_ptr<VariableExprAST>& var);
    std::string getType(const std::unique_ptr<TypedVarAST>& var);
    std::string getType(const std::unique_ptr<ArrayExprAST>& arr);

    const std::unique_ptr<FunctionBaseAST>& getFunc(const std::string& name);

    // Verification functions
    // Return Empty "" String if valid

    bool verifyVar(const std::unique_ptr<VariableExprAST>& var);
    bool verifyVarDef(const std::unique_ptr<VariableDefAST>& var);
    bool verifyTypedVar(const std::unique_ptr<TypedVarAST>& var);
    bool verifyVarAssign(const std::unique_ptr<VariableAssignAST>& var);

    // Constant verification due to overflows and invalid escape sequences
    bool verifyInt(const std::unique_ptr<IntExprAST>& int_); 
    bool verifyFloat(const std::unique_ptr<FloatExprAST>& float_);
    bool verifyDouble(const std::unique_ptr<DoubleExprAST>& double_);
    bool verifyChar(const std::unique_ptr<CharExprAST>& char_);
    bool verifyStr(const std::unique_ptr<StrExprAST>& str);
    bool verifyArray(const std::unique_ptr<ArrayExprAST>& array);
    
    bool verifyFor(const std::unique_ptr<ForExprAST>& for_);
    bool verifyWhile(const std::unique_ptr<WhileExprAST>& while_);
    bool verifyBreak(const std::unique_ptr<BreakExprAST>& break_); // verification needed for break execution statement
    bool verifyContinue(const std::unique_ptr<ContinueExprAST>& continue_); // verification needed for continue execution statements

    bool verifyCall(const std::unique_ptr<CallExprAST>& call);
    bool verifyPrototype(const std::unique_ptr<PrototypeAST>& proto);
    bool verifyProto(std::unique_ptr<PrototypeAST> proto);
    bool verifyExtern(std::unique_ptr<ExternAST> extern_);
    bool verifyFunction(std::unique_ptr<FunctionAST> func);
    bool verifyReturn(const std::unique_ptr<ReturnExprAST>& return_);

    bool verifyUnop(const std::unique_ptr<UnaryExprAST>& unop);
    bool verifyBinop(const std::unique_ptr<BinaryExprAST>& binop);

    bool verifyClass(std::unique_ptr<ClassAST> class_);
    bool verifyNew(const std::unique_ptr<NewExprAST>& new_);
    bool verifyDelete(const std::unique_ptr<DeleteExprAST>& delete_);

    bool verifyUnion(std::unique_ptr<UnionExprAST> union_);
    bool verifyStruct(std::unique_ptr<StructExprAST> struct_);

    bool verifyIf(const std::unique_ptr<IfExprAST>& if_);

    bool verifyUnsafe(const std::unique_ptr<UnsafeAST>& unsafe);
    bool verifyReference(const std::unique_ptr<ReferenceExprAST>& reference);

    bool verifyBlock(std::vector<std::unique_ptr<ExprAST>> const& block);

    bool verifyCode(std::unique_ptr<CodeAST> code);

    bool verifyExpr(const std::unique_ptr<ExprAST>& expr);

    std::vector<std::string> getErrors();
};

}