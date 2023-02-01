#pragma once

#include <iostream>
#include <ostream>
#include <fstream>
#include <string>

namespace vire
{
namespace proto
{
    std::fstream openFile(const std::string& filename);

    std::fstream openFile(const char* filename);

    std::string readFile(std::fstream& file, char close=0);

}
}