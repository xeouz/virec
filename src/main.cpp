#include "vire/includes.hpp"

#include <iostream>
#include <memory>


int entry()
{
    auto api=vire::VApi::loadFromFile("res/test.ve", "wasm32");

    api->parseSourceModule();

    bool s=api->verifySourceModule();
    if(!s) return 1;

    s=api->compileSourceModule("./test.o", true);
    api->getErrorBuilder()->showErrors();
    if(!s) return 1;

    std::cout << "Compiled code" << std::endl;
    std::cout << api->getCompiler()->getCompiledOutput() << std::endl;

    return 0;
}

#ifndef VIRE_USE_EMCC
int main()
{
    int ret=entry();

    return ret;
}
#endif

#ifdef VIRE_USE_EMCC
int main()
{
    return 0;
}
#endif