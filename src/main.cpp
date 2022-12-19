#include "vire/includes.hpp"

#include <iostream>
#include <memory>

int entry()
{
    auto api=vire::VApi::loadFromFile("res/test.ve", "sys");

    api->parseSourceModule();

    bool s=api->verifySourceModule();
    if(!s)
    {
        std::cout << "Verification failed" << std::endl;
        return 1; 
    }

    s=api->compileSourceModule("./test.o", true, vire::Optimization::O0); 
    api->getErrorBuilder()->showErrors();
    if(!s)
    {
        std::cout << "Compilation failed" << std::endl;
        return 1;
    }

    std::cout << "Compiled" << std::endl;
    std::cout << api->getCompiledLLVMIR() << std::endl;
    std::cout << "---" << std::endl;

    return 0;
}

#ifndef VIRE_USE_EMCC
int main()
{
    int ret=0;
    ret=entry();

    return ret;
}
#endif

#ifdef VIRE_USE_EMCC
int main()
{
    return 0;
}
#endif