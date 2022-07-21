// Contains common functions and variables used by parser and compiler
#include "config.hpp"

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
    int Config::getBinopPrecedence(std::string tok)
    {
        int prec=BinopPrecendence[tok];
        if(prec<=0) return -1;
        return prec;
    }
}