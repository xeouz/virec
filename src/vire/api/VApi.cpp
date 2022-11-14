#include "VApi.hpp"

namespace vire
{
void VApi::internal_setup()
{

}

VApi::VApi(std::unique_ptr<VParser> parser, std::unique_ptr<VCompiler> compiler, 
     std::unique_ptr<errors::ErrorBuilder> ebuilder, std::string source_code, std::string target)
: parser(std::move(parser)), compiler(std::move(compiler)), ebuilder(std::move(ebuilder)),
  source_code(source_code), target(target)
{
    internal_setup();
}

VApi::VApi()
{   }

std::unique_ptr<VApi> VApi::loadFromFile(std::string input_file_path, std::string compilation_target)
{
    auto file=vire::proto::openFile(input_file_path);
    std::string src=vire::proto::readFile(file, true);

    auto ebuilder=std::make_unique<errors::ErrorBuilder>("This program");
    auto lexer=std::make_unique<VLexer>(src, ebuilder.get());
    auto parser=std::make_unique<VParser>(std::move(lexer));
    auto analyzer=std::make_unique<VAnalyzer>(ebuilder.get(), src);
    auto compiler=std::make_unique<VCompiler>(std::move(analyzer));

    return std::make_unique<VApi>(std::move(parser), std::move(compiler), std::move(ebuilder), src, compilation_target);
}
std::unique_ptr<VApi> VApi::loadFromText(std::string input_code, std::string compilation_target)
{
    auto ebuilder=std::make_unique<errors::ErrorBuilder>("This program");
    auto lexer=std::make_unique<VLexer>(input_code, ebuilder.get());
    auto parser=std::make_unique<VParser>(std::move(lexer));
    auto analyzer=std::make_unique<VAnalyzer>(ebuilder.get(), input_code);
    auto compiler=std::make_unique<VCompiler>(std::move(analyzer));

    return std::make_unique<VApi>(std::move(parser), std::move(compiler), std::move(ebuilder), input_code, compilation_target); 
}

void VApi::showErrors() const
{
    getErrorBuilder()->showErrors();
}
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
bool VApi::compileSourceModule(std::string output_file_path, bool write_to_file)
{
    std::string out_file_path;

    if(output_file_path=="")
    {
    #ifdef _WIN32
        out_file_path="./a.exe";
    #endif
    #ifndef _WIN32
        out_file_path="./a.out";
    #endif
    }
    else
    {
        out_file_path=output_file_path;
    }

    compiler->compileModule();

    std::string errs;
    llvm::raw_string_ostream os(errs);
    bool failure=llvm::verifyModule(*compiler->getModule(), &os);
    os.flush();
    std::cout << errs << std::endl;
    
    if(!failure && write_to_file)
    {
        compiler->compileToFile(output_file_path, target);
    }
    else if(!failure && !write_to_file)
    {
        byte_output=compiler->compileToString(target);
    }

    return !failure;
}
std::vector<unsigned char> const& VApi::getByteOutput()
{
    return byte_output;
}
std::string const& VApi::getCompiledLLVMIR()
{
    return getCompiler()->getCompiledOutput();
}

void VApi::setSourceCode(std::string new_code)
{
    this->source_code=new_code;
}
void VApi::reset()
{
    this->ast.reset();
    this->compiler->resetModule();
}

#ifdef VIRE_USE_EMCC
using namespace emscripten;

EMSCRIPTEN_BINDINGS(VAPI)
{
    register_vector<unsigned char>("VireVectorUC");

    class_<VApi>("VireAPI")
    .constructor<>()
    .function("ParseSourceModule", &VApi::parseSourceModule)
    .function("VerifySourceModule", &VApi::verifySourceModule)
    .function("CompileSourceModule", &VApi::compileSourceModule)
    .function("getByteOutput", &VApi::getByteOutput)
    .function("getCompiledLLVMIR", &VApi::getCompiledLLVMIR)
    .function("showErrors", &VApi::showErrors)
    .function("setSourceCode", &VApi::setSourceCode)
    .function("reset", &VApi::reset)
    .class_function("loadFromText", &VApi::loadFromText)
    ;
}
#endif

}