#include "builder.hpp"

#include <vector>
#include <string>
#include <iostream>

namespace vire
{
namespace errors
{

    template<>
    void ErrorBuilder::addError<errortypes::lex_unknown_char>(char _char)
    {
        std::string error;
        error.append("Unknown character: ");
        error.push_back(_char);
        this->errors.push_back(error);
    }

    void ErrorBuilder::showErrors()
    {
        for(auto& error : this->errors)
        {
            std::cout << error << std::endl;
        }
    }

}
}