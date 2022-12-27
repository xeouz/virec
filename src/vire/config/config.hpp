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