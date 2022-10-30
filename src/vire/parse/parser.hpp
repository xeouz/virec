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
    std::unique_ptr<Config> config;
    bool parse_success;
public:
    std::unique_ptr<VToken> current_token;
    std::string current_func_name;

    VParser(VLexer* lexer) 
    : lexer(lexer), current_token() {
        config=std::make_unique<Config>();
        config->installDefaultBinops();
    }
    VParser(std::unique_ptr<VLexer> lexer) 
    : lexer(std::move(lexer)), current_token(std::make_unique<VToken>("",tok_eof)) {
        config=std::make_unique<Config>();
        config->installDefaultBinops();
    }
    
    VParser(VLexer* lexer, Config* config)
    : lexer(lexer), config(config), current_token(std::make_unique<VToken>("",tok_eof)) {}
    VParser(std::unique_ptr<VLexer> lexer, Config* config)
    : lexer(std::move(lexer)), config(config), current_token(std::make_unique<VToken>("",tok_eof)) {}
    VParser(std::unique_ptr<VLexer> lexer, std::unique_ptr<Config> config)
    : lexer(std::move(lexer)), config(std::move(config)), current_token(std::make_unique<VToken>("",tok_eof)) {}

    std::unique_ptr<ExprAST> LogError(const char* str,...);
    std::unique_ptr<PrototypeAST> LogErrorP(const char* str,...);
    std::unique_ptr<FunctionAST> LogErrorF(const char* str,...);
    std::unique_ptr<ClassAST> LogErrorC(const char* str,...);
    std::vector<std::unique_ptr<ExprAST>> LogErrorVP(const char* str,...);
    std::unordered_map<std::string, std::unique_ptr<ExprAST>> LogErrorPB(const char* str,...);

    void getNextToken(bool first_token=false);
    void getNextToken(int toktype);
    std::unique_ptr<VToken> copyCurrentToken();

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

    std::unordered_map<std::string, std::unique_ptr<ExprAST>> ParsePrimitiveBody();
    std::unique_ptr<ExprAST> ParseUnion();
    std::unique_ptr<ExprAST> ParseStruct();

    std::unique_ptr<ExprAST> ParseUnsafe();
    std::unique_ptr<ExprAST> ParseReference();

    std::unique_ptr<ModuleAST> ParseSourceModule();
};

}