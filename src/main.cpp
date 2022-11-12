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
        std::cout << "Verification Failed" << std::endl;
        return 1; 
    }

    s=api->compileSourceModule("./test.o", true);
    api->getErrorBuilder()->showErrors();
    if(!s)
    {
        std::cout << "Compilation Failed" << std::endl;
        return 1;
    }

    std::cout << "Compiled code" << std::endl;
    std::cout << api->getCompiler()->getCompiledOutput() << std::endl;
 
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