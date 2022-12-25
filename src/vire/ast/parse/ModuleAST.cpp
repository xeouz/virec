#pragma once

#include "ASTType.hpp"
#include "ExprAST.cpp"

#include "FunctionAST.cpp"
#include "ClassAST.cpp"
#include "UnionStructAST.cpp"

#include <vector>
#include <memory>

namespace vire
{

class ModuleAST
{
    std::vector<VariableDefAST*> PreExecutionStatementsVariables;
    std::vector<FunctionAST*> Constructors;
    std::vector<std::unique_ptr<ExprAST>> PreExecutionStatements;
    std::vector<std::unique_ptr<FunctionBaseAST>> Functions;
    std::vector<std::unique_ptr<ClassAST>> Classes;
    std::vector<std::unique_ptr<ExprAST>> UnionStructs;
public:
    ModuleAST(std::vector<std::unique_ptr<ExprAST>> PreExecutionStatements,
            std::vector<std::unique_ptr<FunctionBaseAST>> Functions,
            std::vector<std::unique_ptr<ClassAST>> Classes,
            std::vector<std::unique_ptr<ExprAST>> UnionStructs)
    :   PreExecutionStatements(std::move(PreExecutionStatements)),
        Functions(std::move(Functions)),
        Classes(std::move(Classes)),
        UnionStructs(std::move(UnionStructs)) {}

    std::vector<std::unique_ptr<ExprAST>> const& getPreExecutionStatements() const {
        return PreExecutionStatements;
    }
    std::vector<std::unique_ptr<FunctionBaseAST>> const& getFunctions() const {
        return Functions;
    }
    std::vector<std::unique_ptr<ClassAST>> const& getClasses() const {
        return Classes;
    }
    std::vector<std::unique_ptr<ExprAST>> const& getUnionStructs() const {
        return UnionStructs;
    }
    std::vector<VariableDefAST*> const& getPreExecutionStatementsVariables() const {
        return PreExecutionStatementsVariables;
    }
    std::vector<FunctionAST*> const& getConstructors() const {
        return Constructors;
    }

    std::vector<std::unique_ptr<ExprAST>> movePreExecutionStatements() {
        return std::move(PreExecutionStatements);
    }
    std::vector<std::unique_ptr<FunctionBaseAST>> moveFunctions() {
        return std::move(Functions);
    }
    std::vector<std::unique_ptr<ClassAST>> moveClasses() {
        return std::move(Classes);
    }
    std::vector<std::unique_ptr<ExprAST>> moveUnionStructs() {
        return std::move(UnionStructs);
    }
    std::vector<FunctionAST*> moveConstructors() {
        return Constructors;
    }

    void addFunction(std::unique_ptr<FunctionBaseAST> func) {
        Functions.push_back(std::move(func));
    }
    void addConstructor(FunctionAST* constructor) {
        Constructors.push_back(constructor);
    }

    void addUnionStruct(std::unique_ptr<ExprAST> union_struct)
    {
        UnionStructs.push_back(std::move(union_struct));
    }

    void addPreExecutionStatements(std::vector<std::unique_ptr<ExprAST>> stms){
        this->PreExecutionStatements.reserve(this->PreExecutionStatements.size() + stms.size());
        this->PreExecutionStatements.insert(this->PreExecutionStatements.end(), std::make_move_iterator(stms.begin()), std::make_move_iterator(stms.end()));
    }

    void addPreExecutionStatementVariables(std::vector<VariableDefAST*> const& vars)
    {
        this->PreExecutionStatementsVariables.reserve(this->PreExecutionStatementsVariables.size() + vars.size());
        this->PreExecutionStatementsVariables.insert(this->PreExecutionStatementsVariables.end(), vars.begin(), vars.end());
    }

    void addConstructors(std::vector<FunctionAST*> const& consts)
    {
        this->Constructors.reserve(this->Constructors.size() + consts.size());
        this->Constructors.insert(this->Constructors.end(), consts.begin(), consts.end());
    }
};

}