#include "builder.hpp"

#include <vector>
#include <string>
#include <iostream>

namespace vire
{
namespace errors
{
    #define bold_tag "\033[1m"
    #define dull_tag "\033[2m"
    #define reset_tag "\033[0m"

    #define red_tag "\033[31m"
    #define green_tag "\033[32m"
    #define yellow_tag "\033[33m"
    #define blue_tag "\033[34m"
    #define magenta_tag "\033[35m"
    #define cyan_tag "\033[36m"
    #define white_tag "\033[37m"


    std::string ErrorBuilder::constructCodePosition
    (const std::string& input, std::size_t line, std::size_t column, int column_len)
    {
        std::string result;
        
        std::string code(input);
        std::vector<std::string> lines;
        std::size_t pos=0;
        while((pos=code.find("\n"))!=std::string::npos)
        {
            lines.push_back(code.substr(0, pos));
            code.erase(0, pos+1);
        }

        int start_pos=0;
        int end_pos=line+1;
        if(line>2)  start_pos=end_pos-3;

        for(int i=start_pos; i<end_pos; ++i)
        {
            result+=bold_tag;
            result+=std::to_string(i+1);
            result+=" |  ";
            result+=reset_tag;

            if(i!=end_pos-1)
            {
                result+=dull_tag;
                result+=lines[i];
                result+=reset_tag;
            }
            else
            {
                result+=red_tag;
                result+=lines[i];
                result+=reset_tag;
            }
            result+="\n";
        }
        
        for(int i=0; i<column+5; ++i) result+=" ";
        result+=red_tag;
        for(int i=0; i<column_len; ++i) result+="^";
        result+=reset_tag;

        std::cout << result << std::endl;

        return "";
    } 

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