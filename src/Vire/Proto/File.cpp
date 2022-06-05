#include <iostream>
#include <fstream>
#include <string>

#include "File.hpp"

namespace vire
{
namespace proto
{

    std::fstream openFile(const std::string& filename)
    {
        std::fstream file(filename);

        if(!file.good())
        {
            std::cout << "File " << filename << " doest not exist." << std::endl;
        }

        return file;
    }

    std::fstream openFile(const char* filename)
    {
        std::fstream file(filename);

        if(!file.good())
        {
            std::cout << "File " << filename << " doest not exist." << std::endl;
        }

        return file;
    }

    std::string readFile(std::fstream& file, char close)
    {
        std::string buf, out;

        while(std::getline(file,buf))
        {
            out += buf;
            out += '\n';
        }

        if(close)
            file.close();

        return out;
    }

}
}