#pragma once

#include <map>
#include <string>
#include <array>

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