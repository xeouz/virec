#pragma once

#include <unordered_map>
#include <string>
#include <array>

#ifdef VIRE_USE_EMCC
#define VIRE_ENABLE_ONLY WebAssembly
#endif

#ifndef VIRE_USE_EMCC
#define VIRE_ENABLE_ONLY X86
#endif

#include "vire/lex/token.hpp"
#include "vire/parse/keyword_hash.hpp"

namespace vire
{

enum class Optimization
{
    O0,
    O1,
    O2,
    O3,
    Os,
    Oz,
};

inline std::unordered_map<std::string, Optimization> str_to_optimization=
{
    {"O0", Optimization::O0},
    {"O1", Optimization::O1},
    {"O2", Optimization::O2},
    {"O3", Optimization::O3},
    {"Os", Optimization::Os},
    {"Oz", Optimization::Oz},
};

inline std::unordered_map<Optimization, std::string> optimization_to_str=
{
    {Optimization::O0, "O0"},
    {Optimization::O1, "O1"},
    {Optimization::O2, "O2"},
    {Optimization::O3, "O3"},
    {Optimization::Os, "Os"},
    {Optimization::Oz, "Oz"},
};

class Config
{
public:
    std::unordered_map<std::string, int> BinopPrecendence; // used by parser
    std::unordered_map<std::string, int> KeywordTokenMap; // used by lexer
    std::unordered_map<std::string, int>::iterator KeywordTokenMap_end;
    
    void installDefaultBinops(); // default binops
    void installDefaultKeywords(); // default keywords

    int getBinopPrecedence(std::string const& tok);
    int getKeywordToken(std::string const& keyw);
};

}