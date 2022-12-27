#pragma once

#include "vire/ast/include.hpp"
#include "vire/lex/include.hpp"
#include "vire/config/include.hpp"

#include <memory>
#include <string>
#include <vector>
#include <cstdarg>
#include <map>

namespace vire
{

class VParser
{
    std::unique_ptr<VLexer> lexer;
    Config* config;
    bool parse_success;
public:
    std::unique_ptr<VToken> current_token;
    const proto::IName* current_func_name;

    VParser(VLexer* _lexer, Config* _config=nullptr)
    : lexer(_lexer), current_token() {
        if(_config) config=_config;
        else config=lexer->getConfig();
    }
    VParser(std::unique_ptr<VLexer> _lexer, Config* _config=nullptr) 
    : lexer(std::move(_lexer)), current_token(std::make_unique<VToken>("",tok_eof)) {
        if(_config) config=_config;
        else config=lexer->getConfig();
    }
    
    std::unique_ptr<ExprAST> LogError(const char* str,...);
    std::unique_ptr<PrototypeAST> LogErrorP(const char* str,...);
    std::unique_ptr<FunctionAST> LogErrorF(const char* str,...);
    std::unique_ptr<ClassAST> LogErrorC(const char* str,...);
    std::vector<std::unique_ptr<ExprAST>> LogErrorVP(const char* str,...);
    std::pair<std::unordered_map<proto::IName, std::unique_ptr<ExprAST>>, std::unique_ptr<FunctionAST>> LogErrorPB(const char* str,...);

    void getNextToken(bool first_token=false);
    void getNextToken(int toktype);
    std::unique_ptr<VToken> copyCurrentToken();

    std::unique_ptr<types::Base> ParseTypeIdentifier();
    std::vector<std::unique_ptr<ExprAST>> ParseBlock();

    std::unique_ptr<ExprAST> ParsePrimary();
    std::unique_ptr<ExprAST> ParseExpression();

    std::unique_ptr<ExprAST> ParseIdExpr(bool include_assign=true);
    std::unique_ptr<ExprAST> ParseIncrementDecrementExpr();

    std::unique_ptr<ExprAST> ParseNumberExpr();
    std::unique_ptr<ExprAST> ParseStrExpr();
    std::unique_ptr<ExprAST> ParseBoolExpr();
    std::unique_ptr<ExprAST> ParseArrayExpr();

    std::unique_ptr<ExprAST> ParseParenExpr();

    std::unique_ptr<ExprAST> ParseBinopExpr(int ExprPrec, std::unique_ptr<ExprAST> LHS);

    std::unique_ptr<ExprAST> ParseVariableDef();
    std::unique_ptr<ExprAST> ParseVariableAssign(std::unique_ptr<ExprAST> expr);
    std::unique_ptr<ExprAST> ParseShorthandVariableAssign(std::unique_ptr<ExprAST> expr);

    std::unique_ptr<ExprAST> ParseForExpr();
    std::unique_ptr<ExprAST> ParseWhileExpr();
    std::unique_ptr<ExprAST> ParseBreakContinue();

    std::unique_ptr<PrototypeAST> ParsePrototype();

    std::unique_ptr<PrototypeAST> ParseProto();
    std::unique_ptr<ExternAST> ParseExtern();
    std::unique_ptr<FunctionAST> ParseFunction();
    std::unique_ptr<ExprAST> ParseReturn();

    std::unique_ptr<ExprAST> ParseIfExpr();

    std::unique_ptr<ClassAST> ParseClass();
    std::unique_ptr<ExprAST> ParseNewExpr();
    std::unique_ptr<ExprAST> ParseDeleteExpr();
    std::unique_ptr<ExprAST> ParseClassAccess(std::unique_ptr<ExprAST> parent);

    std::unique_ptr<FunctionAST> ParseConstructor();
    std::pair<std::unordered_map<proto::IName, std::unique_ptr<ExprAST>>, std::unique_ptr<FunctionAST>> ParsePrimitiveBody();
    std::unique_ptr<ExprAST> ParseUnion();
    std::unique_ptr<ExprAST> ParseStruct();

    std::unique_ptr<ExprAST> ParseUnsafe();
    std::unique_ptr<ExprAST> ParseReference();

    std::unique_ptr<ModuleAST> ParseSourceModule();
};

}