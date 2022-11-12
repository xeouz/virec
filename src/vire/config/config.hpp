#pragma once

#include <map>
#include <string>
#include <array>

#ifdef VIRE_USE_EMCC
#define VIRE_ENABLE_ONLY WebAssembly
#endif

#ifndef VIRE_USE_EMCC
#define VIRE_ENABLE_ONLY X86
#endif

namespace vire
{

class Config
{
public:
    std::map<std::string, int> BinopPrecendence; // used by parser
    
    void installDefaultBinops(); // default binops
    int getBinopPrecedence(std::string tok);
};

}