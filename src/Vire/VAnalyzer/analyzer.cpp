#include "analyzer.hpp"

#include "../AST-Parse/Include.hpp"
#include "../AST-Defs/Include.hpp"
#include "../Error-Builder/Include.hpp"

#include <string>
#include <memory>
#include <vector>
#include <algorithm>

namespace vire
{
    bool VAnalyzer::isVarDefined(const std::string& name)
    {
        if(!variables.empty())
        {
            for(int i=0; i<variables.size(); i++)
            {
                if(variables[i]->getName()==name)
                {
                    return true;
                }
            }
        }
        return false;
    }

    // Verification Functions
    std::string VAnalyzer::verifyVar(const std::unique_ptr<VariableExprAST>& var)
    {
        // Check if it is defined
        if(!isVarDefined(var->getName()))
        {
            // Var is not defined
            return "";
        }
        return "";
    }
    std::string VAnalyzer::verifyVarDef(const std::unique_ptr<VariableDefAST>& var)
    {
        if(!isVarDefined(var->getName()))
        {
            if(var->isLet() || var->isConst())
            {
                if(var->getValue() == nullptr && var->getType() == "auto")
                {
                    // Requires a variable for definiton
                    std::cout << "Error: " << var->getName() << " is not defined" << std::endl;
                }
            }
        }
        // Variable is already defined
        return "";
    }

}