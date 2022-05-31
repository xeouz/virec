#pragma once

#include <iostream>
#include <fstream>
#include <string>

namespace vire
{
namespace proto
{
    static std::fstream openFile(const std::string& filename)
    {
        std::fstream file(filename);

        if(!file.good())
        {
            std::cout << "File " << filename << " doest not exist." << std::endl;
        }

        return file;
    }

    static std::fstream openFile(const char* filename)
    {
        std::fstream file(filename);

        if(!file.good())
        {
            std::cout << "File " << filename << " doest not exist." << std::endl;
        }

        return file;
    }

    static std::string readFile(std::fstream& file, char close=0)
    {
        std::string buf, out;

        while(std::getline(file,buf))
        {
            out += buf;
        }

        if(close)
            file.close();

        return out;
    }
}
}