#include "VApi.hpp"

namespace vire
{

errors::ErrorBuilder* const VApi::getErrorBuilder() const
{
    return ebuilder.get();
}

VCompiler* const VApi::getCompiler() const
{
    return compiler.get();
}

bool VApi::parseSourceModule()
{
    ast=parser->ParseSourceModule();

    if(!ast)
    {
        return 0;
    }
    return 1;
}
bool VApi::verifySourceModule()
{
    bool success=compiler->getAnalyzer()->verifySourceModule(std::move(ast));
    return success;
}
bool VApi::compileSourceModule()
{
    compiler->compileModule();
    bool failure=llvm::verifyModule(*compiler->getModule());
    
    if(!failure)
    {
        compiler->compileToObjectFile(out_file_name, target);
    }

    return !failure;
}

}