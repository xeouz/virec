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
    std::unique_ptr<CodeAST> codeast;

    FunctionAST* current_func;

    // Error Builder
    errors::ErrorBuilder* const& builder;

    // Source Code
    std::string code;

    // Scope Stack
    std::map<std::string, VariableDefAST*> scope;

    // Functions
    void addVar(VariableDefAST* const& var);
    void removeVar(VariableDefAST* const& var);
    void addFunction(std::unique_ptr<FunctionBaseAST> func);
    void addClass(std::unique_ptr<ClassAST> class_);
    bool isVarDefined(std::string const& name, bool check_globally_only=false);

    VariableDefAST* const getVar(std::string const& name);
public:
    VAnalyzer(errors::ErrorBuilder* const& builder, std::string const& code="")
    : builder(builder), code(code) {}

    bool isStructDefined(std::string const& name);
    bool isUnionDefined(std::string const& name);
    bool isClassDefined(std::string const& name);
    bool isFuncDefined(std::string const& name);
    bool isStdFunc(std::string const& name);
    unsigned int getFuncArgCount(std::string const& name);

    std::string getType(ExprAST* const& expr);
    std::string getType(VariableExprAST* const& var);
    std::string getType(TypedVarAST* const& var);
    std::string getType(ArrayExprAST* const& arr);

    FunctionBaseAST* const getFunc(const std::string& name);
    std::string const& getFuncReturnType(const std::string& name="");

    CodeAST* const getCode();

    // Verification functions
    // Return Empty "" String if valid
    ReturnExprAST* const getReturnStatement(std::vector<std::unique_ptr<ExprAST>> const& block);

    bool verifyVar(VariableExprAST* const& var);
    bool verifyIncrDecr(VariableIncrDecrAST* const& var);
    bool verifyVarDef(VariableDefAST* const& var, bool check_globally_only=false);
    bool verifyTypedVar(TypedVarAST* const& var);
    bool verifyVarAssign(VariableAssignAST* const& var);

    // Constant verification due to overflows and invalid escape sequences
    bool verifyInt(IntExprAST* const& int_); 
    bool verifyFloat(FloatExprAST* const& float_);
    bool verifyDouble(DoubleExprAST* const& double_);
    bool verifyChar(CharExprAST* const& char_);
    bool verifyStr(StrExprAST* const& str);
    bool verifyArray(ArrayExprAST* const& array);
    
    bool verifyFor(ForExprAST* const& for_);
    bool verifyWhile(WhileExprAST* const& while_);
    bool verifyBreak(BreakExprAST* const& break_); // verification needed for break execution statement
    bool verifyContinue(ContinueExprAST* const& continue_); // verification needed for continue execution statements

    bool verifyCall(CallExprAST* const& call);
    bool verifyPrototype(PrototypeAST* const& proto);
    bool verifyProto(PrototypeAST* const& proto);
    bool verifyExtern(ExternAST* const& extern_);
    bool verifyFunction(FunctionAST* const& func);
    bool verifyReturn(ReturnExprAST* const& return_);

    bool verifyUnop(UnaryExprAST* const& unop);
    bool verifyBinop(BinaryExprAST* const& binop);

    bool verifyClass(ClassAST* const& class_);
    bool verifyNew(NewExprAST* const& new_);
    bool verifyDelete(DeleteExprAST const& delete_);

    bool verifyUnionStructBody(std::vector<std::unique_ptr<ExprAST>> const& body);
    bool verifyUnion(UnionExprAST* const& union_);
    bool verifyStruct(StructExprAST* const& struct_);

    bool verifyIfThen(IfThenExpr* const& if_);
    bool verifyIf(IfExprAST* const& if_);

    bool verifyUnsafe(UnsafeExprAST* const& unsafe);
    bool verifyReference(ReferenceExprAST* const& reference);

    bool verifyBlock(std::vector<std::unique_ptr<ExprAST>> const& block);

    bool verifyCode(std::unique_ptr<CodeAST> code);

    bool verifyExpr(ExprAST* const& expr);

    std::vector<std::string> getErrors();
};

}