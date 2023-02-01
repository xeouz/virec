#pragma once

#include "vire/parse/include.hpp"
#include "vire/ast/include.hpp"
#include "vire/errors/include.hpp"
#include "vire/proto/iname.hpp"

namespace vire
{

class VAnalyzer
{
    // Symbol Tables
    std::unique_ptr<ModuleAST> ast;

    FunctionAST* current_func;
    StructExprAST* current_struct;

    // Error Builder
    errors::ErrorBuilder* const builder;

    // Source Code
    std::string code;

    // Scope Stack
    std::map<std::string, VariableDefAST*> scope;
    std::vector<VariableDefAST*>* scope_varref;

    // Type Stack
    std::map<std::string, ExprAST*> types;

    // Functions
    void defineVariable(VariableDefAST* const var, bool is_arg=false);
    void undefineVariable(VariableDefAST* const var);
    void undefineVariable(proto::IName const& var);

    void addFunction(std::unique_ptr<FunctionBaseAST> func);
    void addConstructor(FunctionAST* constructor);
    void addClass(std::unique_ptr<ClassAST> class_);
    bool isVariableDefined(proto::IName const& name);

    VariableDefAST* const getVariable(std::string const& name);
    VariableDefAST* const getVariable(proto::IName const& name);
public:
    VAnalyzer(errors::ErrorBuilder* const builder, std::string const& code="")
    : builder(builder), code(code), scope_varref(nullptr), current_func(nullptr), current_struct(nullptr) {}

    errors::ErrorBuilder* const getErrorBuilder() const { return builder; }

    bool isStructDefined(std::string const& name);
    bool isUnionDefined(std::string const& name);
    bool isClassDefined(std::string const& name);
    bool isFunctionDefined(std::string const& name);
    unsigned int getFunctionArgCount(std::string const& name);

    types::Base* getType(ExprAST* const expr);
    types::Base* getType(ArrayExprAST* const arr);
    std::unique_ptr<types::Base> moveFuncReturnType(const std::string& name="");

    FunctionBaseAST* const getFunction(const std::string& name);
    StructExprAST* const getStruct(const std::string& name);
    FunctionBaseAST* const getFunction(const proto::IName& name);
    StructExprAST* const getStruct(const proto::IName& name);

    ModuleAST* const getSourceModule();

    ///- Verification functions -///
    ReturnExprAST* const getReturnStatement(std::vector<std::unique_ptr<ExprAST>> const& block);
    std::unique_ptr<ExprAST> tryCreateImplicitCast(types::Base* t1, types::Base* t2, std::unique_ptr<ExprAST> expr);

    // Variable related varifications
    bool verifyVariable(VariableExprAST* const var);
    bool verifyIncrementDecrement(IncrementDecrementAST* const incrdecr);
    bool verifyVariableDefinition(VariableDefAST* const var, bool add_to_scope=true);
    bool verifyVarAssign(VariableAssignAST* const var);
    bool verifyVarArrayAccess(VariableArrayAccessAST* const access);

    // Constant/Literal verification due to overflows and invalid escape sequences
    bool verifyInt(IntExprAST* const int_); 
    bool verifyFloat(FloatExprAST* const float_);
    bool verifyDouble(DoubleExprAST* const double_);
    bool verifyChar(CharExprAST* const char_);
    bool verifyStr(StrExprAST* const str);
    bool verifyBool(BoolExprAST* const bool_);
    bool verifyArray(ArrayExprAST* const array);
    
    // Loop verifications
    bool verifyFor(ForExprAST* const for_);
    bool verifyWhile(WhileExprAST* const while_);
    bool verifyBreak(BreakExprAST* const break_); // verification needed for break execution statement
    bool verifyContinue(ContinueExprAST* const continue_); // verification needed for continue execution statements
    
    // Function verifications
    bool verifyCall(CallExprAST* const call);
    bool verifyPrototype(PrototypeAST* const proto);
    bool verifyProto(PrototypeAST* const proto);
    bool verifyExtern(ExternAST* const extern_);
    bool verifyFunction(FunctionAST* const func);
    bool verifyReturn(ReturnExprAST* const return_);

    // Operator and Cast verifications
    bool verifyUnop(UnaryExprAST* const unop);
    bool verifyBinop(BinaryExprAST* const binop);
    bool verifyCastExpr(CastExprAST* const cast);

    // Class verifications
    bool verifyClass(ClassAST* const class_);
    bool verifyNew(NewExprAST* const new_);
    bool verifyDelete(DeleteExprAST const& delete_);

    // Struct/Union verifications
    bool verifyUnionStructBody(std::vector<ExprAST*> const& body);
    bool verifyUnion(UnionExprAST* const union_);
    bool verifyStruct(StructExprAST* const struct_);
    bool verifyTypeAccess(TypeAccessAST* const member);

    // If-Else verifications
    bool verifyIfThen(IfThenExpr* const if_);
    bool verifyIf(IfExprAST* const if_);

    // Memory-related verifications
    bool verifyUnsafe(UnsafeExprAST* const unsafe);
    bool verifyReference(ReferenceExprAST* const reference);

    // Block verifications
    bool verifyBlock(std::vector<std::unique_ptr<ExprAST>> const& block);

    // Entry point for verification
    bool verifySourceModule(std::unique_ptr<ModuleAST> code);

    // Helper functions
    bool verifyExpr(ExprAST* const expr);

    std::vector<std::string> getErrors();
};

}