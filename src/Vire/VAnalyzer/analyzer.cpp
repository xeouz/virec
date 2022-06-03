#include "analyzer.hpp"

#include "../AST-Parse/Include.hpp"
#include "../AST-Defs/Include.hpp"

namespace vire
{
    bool VAnalyzer::isVarDefined(const std::string& name)
    {
        return false;
    }

    bool VAnalyzer::verifyVar(std::unique_ptr<VariableExprAST> var)
    {
        // Check if it is defined
        if(!isVarDefined(var->getName()))
        {
            // Var is not defined
            return false;
        }
        return true;
    }
    bool VAnalyzer::verifyVarDef(const std::unique_ptr<VariableDefAST>& var)
    {
        std::cout << "Hello there from vardef analyzer " << var->getName() << std::endl;

        return false;
    }

}