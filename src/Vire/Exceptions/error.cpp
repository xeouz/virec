#pragma once

#include <iostream>
#include <memory>
#include <cstdlib>

void LogExc(const char* str)
{
    std::cout<<str<<std::endl;
    exit(1);
}