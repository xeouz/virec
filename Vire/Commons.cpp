#pragma once 

// Contains common functions and variables used by parser and compiler

#include <map>
#include <string>
#include <array>

namespace vire
{
class Commons
{
public:
    std::map<std::string, int> BinopPrecendence; // used by parser
    
    void installDefaultBinops() // default binops
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
    int getBinopPrecedence(std::string tok)
    {
        int prec=BinopPrecendence[tok];
        if(prec<=0) return -1;
        return prec;
    }
};
}