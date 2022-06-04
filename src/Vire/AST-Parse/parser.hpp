#pragma once

#include "../AST-Defs/Include.hpp"
#include "../Lex/Include.hpp"
#include "../Commons.cpp"

#include <memory>
#include <string>
#include <vector>
#include <cstdarg>

namespace vire
{

class Vireparse
{
    std::unique_ptr<Virelex> lexer;
    std::unique_ptr<Commons> config;
public:
    std::unique_ptr<Viretoken> CurTok;

    Vireparse(Virelex* lexer) 
    : lexer(lexer), CurTok() {
        config=std::make_unique<Commons>();
        config->installDefaultBinops();
    }
    Vireparse(std::unique_ptr<Virelex> lexer) 
    : lexer(std::move(lexer)), CurTok(std::make_unique<Viretoken>("",tok_eof)) {
        config=std::make_unique<Commons>();
        config->installDefaultBinops();
    }
    
    Vireparse(Virelex* lexer, Commons* config)
    : lexer(lexer), config(config), CurTok(std::make_unique<Viretoken>("",tok_eof)) {}
    Vireparse(std::unique_ptr<Virelex> lexer, Commons* config)
    : lexer(std::move(lexer)), config(config), CurTok(std::make_unique<Viretoken>("",tok_eof)) {}
    Vireparse(std::unique_ptr<Virelex> lexer, std::unique_ptr<Commons> config)
    : lexer(std::move(lexer)), config(std::move(config)), CurTok(std::make_unique<Viretoken>("",tok_eof)) {}


    std::unique_ptr<ExprAST> LogError(const char* str,...);
    std::unique_ptr<PrototypeAST> LogErrorP(const char* str,...);
    std::unique_ptr<FunctionAST> LogErrorF(const char* str,...);
    std::unique_ptr<ClassAST> LogErrorC(const char* str,...);
    std::vector<std::unique_ptr<ExprAST>> LogErrorVP(const char* str,...);

    void getNextToken();
    void getNextToken(int toktype);
    std::unique_ptr<Viretoken> copyCurrentToken();

    std::vector<std::unique_ptr<ExprAST>> ParseBlock();

    std::unique_ptr<FunctionAST> ParseTopLevelExpr();
    std::unique_ptr<ExprAST> ParsePrimary();
    std::unique_ptr<ExprAST> ParseExpression();

    std::unique_ptr<ExprAST> ParseIdExpr();

    std::unique_ptr<ExprAST> ParseNumberExpr();
    std::unique_ptr<ExprAST> ParseStrExpr();
    std::unique_ptr<ExprAST> ParseArrayExpr();

    std::unique_ptr<ExprAST> ParseParenExpr();

    std::unique_ptr<ExprAST> ParseBinopExpr(int ExprPrec, std::unique_ptr<ExprAST> LHS);

    std::unique_ptr<ExprAST> ParseVariableDef();
    std::unique_ptr<ExprAST> ParseVariableAssign(std::unique_ptr<Viretoken> varName);
    std::unique_ptr<TypedVarAST> ParseTypedVar();

    std::unique_ptr<ExprAST> ParseForExpr();
    std::unique_ptr<ExprAST> ParseWhileExpr();
    std::unique_ptr<ExprAST> ParseBreakContinue();

    std::unique_ptr<PrototypeAST> ParsePrototype();

    std::unique_ptr<PrototypeAST> ParseProto();
    std::unique_ptr<ExternAST> ParseExtern();
    std::unique_ptr<FunctionAST> ParseFunction();
    std::unique_ptr<ExprAST> ParseReturn();

    std::unique_ptr<ClassAST> ParseClass();
    std::unique_ptr<ExprAST> ParseNewExpr();
    std::unique_ptr<ExprAST> ParseDeleteExpr();
    std::unique_ptr<ExprAST> ParseClassAccess(std::unique_ptr<ExprAST> parent);

    std::vector<std::unique_ptr<ExprAST>> ParseNativeBody();
    std::unique_ptr<ExprAST> ParseUnion();
    std::unique_ptr<ExprAST> ParseStruct();

    std::unique_ptr<ExprAST> ParseUnsafe();
    std::unique_ptr<ExprAST> ParseReference();
};

}