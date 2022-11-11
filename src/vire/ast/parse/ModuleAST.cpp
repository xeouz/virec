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

    void addFunction(std::unique_ptr<FunctionBaseAST> func) {
        Functions.push_back(std::move(func));
    }

    void addUnionStruct(std::unique_ptr<ExprAST> union_struct)
    {
        UnionStructs.push_back(std::move(union_struct));
    }

    void addPreExecutionStatements(std::vector<std::unique_ptr<ExprAST>> stms){
        int pes_size=PreExecutionStatements.size();
        PreExecutionStatements.resize(pes_size+stms.size());
        for(size_t i=0; i<stms.size(); ++i)
        {
            PreExecutionStatements[pes_size+i]=std::move(stms[i]);
        }
    }
};

}