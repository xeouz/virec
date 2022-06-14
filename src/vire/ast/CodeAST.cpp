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

class CodeAST{
    std::vector<std::unique_ptr<ExprAST>> PreExecutionStatements;
    std::vector<std::unique_ptr<FunctionBaseAST>> Functions;
    std::vector<std::unique_ptr<ClassAST>> Classes;
    std::vector<std::unique_ptr<ExprAST>> UnionStructs;
public:
    CodeAST(std::vector<std::unique_ptr<ExprAST>> PreExecutionStatements,
            std::vector<std::unique_ptr<FunctionBaseAST>> Functions,
            std::vector<std::unique_ptr<ClassAST>> Classes,
            std::vector<std::unique_ptr<ExprAST>> UnionStructs)
    :   PreExecutionStatements(std::move(PreExecutionStatements)),
        Functions(std::move(Functions)),
        Classes(std::move(Classes)),
        UnionStructs(std::move(UnionStructs)) {}

    //std::vector<std::unique_ptr<ExprAST>> getPreExecutionStatements() {return;}
};

}