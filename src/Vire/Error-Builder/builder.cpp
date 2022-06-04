#include "builder.hpp"

#include <vector>
#include <string>
#include <iostream>

namespace vire
{
namespace errors
{
    #define bold_tag "\033[1m"
    #define reset_tag "\033[0m"

    #define red_tag "\033[31m"
    #define green_tag "\033[32m"
    #define yellow_tag "\033[33m"
    #define blue_tag "\033[34m"
    #define magenta_tag "\033[35m"
    #define cyan_tag "\033[36m"
    #define white_tag "\033[37m"

    template<>
    void ErrorBuilder::addError<lex_unknown_char>(char _char, char fix)
    {
        std::string error;
        error.append(red_tag);
        error.append(this->prefix);
        error.append(" has found an unexpected character.\n");
        error.append(this->prefix);
        error.append(" found an unknown character `");
        error.append(bold_tag);
        error.append(1, _char);
        error.append(reset_tag);
        error.append(red_tag);
        error.append("`.\n");

        if(fix!='\0')
        {
            error.append(magenta_tag);
            error.append("\n");
            error.append("You can fix this by replacing the character with `");
            error.append(bold_tag);
            error.append(1, fix);
            error.append(reset_tag);
            error.append(magenta_tag);
            error.append("`.\n");
        }

        error.append(reset_tag);
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