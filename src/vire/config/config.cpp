// Contains common functions and variables used by parser and compiler
#include "config.hpp"
#include <iostream>
#include <ostream>

namespace vire
{
    void Config::installDefaultBinops() // default binops
    {
        BinopPrecendence["<"]=10;
        BinopPrecendence[">"]=10;
        BinopPrecendence["<="]=10;
        BinopPrecendence[">="]=10;
        BinopPrecendence["=="]=10;
        BinopPrecendence["!="]=10;
        BinopPrecendence["+"]=20;
        BinopPrecendence["-"]=20;
        BinopPrecendence["*"]=40;
        BinopPrecendence["/"]=40;
        BinopPrecendence["%"]=40;
        BinopPrecendence["**"]=60;
    }
    void Config::installDefaultKeywords() // default keywords
    {
        KeywordTokenMap["func"]=tok_func;
        KeywordTokenMap["and"]=tok_and;
        KeywordTokenMap["or"]=tok_or;
        KeywordTokenMap["if"]=tok_if;
        KeywordTokenMap["else"]=tok_else;
        KeywordTokenMap["var"]=tok_var;
        KeywordTokenMap["let"]=tok_let;
        KeywordTokenMap["const"]=tok_const;
        KeywordTokenMap["true"]=tok_true;
        KeywordTokenMap["false"]=tok_false;
        KeywordTokenMap["as"]=tok_as;
        KeywordTokenMap["new"]=tok_new;
        KeywordTokenMap["delete"]=tok_delete;
        KeywordTokenMap["class"]=tok_class;
        KeywordTokenMap["union"]=tok_union;
        KeywordTokenMap["struct"]=tok_struct;
        KeywordTokenMap["extern"]=tok_extern;
        KeywordTokenMap["for"]=tok_for;
        KeywordTokenMap["while"]=tok_while;
        KeywordTokenMap["return"]=tok_return;
        KeywordTokenMap["break"]=tok_break;
        KeywordTokenMap["continue"]=tok_continue;
        KeywordTokenMap["returns"]=tok_returns;
        KeywordTokenMap["proto"]=tok_proto;
        KeywordTokenMap["extends"]=tok_extends;
        KeywordTokenMap["try"]=tok_try;
        KeywordTokenMap["except"]=tok_except;
        KeywordTokenMap["unsafe"]=tok_unsafe;
        KeywordTokenMap["constructor"]=tok_constructor;
        KeywordTokenMap_end=KeywordTokenMap.end();
    }
    
    int Config::getBinopPrecedence(std::string const& tok)
    {
        if(BinopPrecendence.contains(tok))
            return BinopPrecendence.at(tok);
        else
            return -1;
    }
    int Config::getKeywordToken(std::string const& keyw)
    {
        auto* keyword=Perfect_Hash::hash_keyword_to_token(keyw.c_str(), keyw.length());
        if(keyword)
            return keyword->KeywordCode;

        return tok_id;
    }
}