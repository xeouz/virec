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
        
        for(int i=0; i<column+3; ++i) result+=" ";
        result+=red_tag;
        for(int i=0; i<column_len; ++i) result+="^";
        result+=reset_tag;

        return result;
    } 

    template<>
    void ErrorBuilder::addError<lex_unknown_char>(const std::string& code, char _char, char fix, std::size_t line, std::size_t column)
    {
        std::string error;
        error+=constructCodePosition(code, line, column, 1);
        error+="\n";

        error.append(red_tag);
        error.append(this->prefix);
        error.append(" has found an unexpected character.\n");
        error.append(this->prefix);
        error.append(" found an unknown character `");
        error.append(bold_tag);
        if(_char==' ')  error.append("End-of-file");
        else    error.append(1, _char);
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
    
    template<>
    void ErrorBuilder::addError<analyze_requires_type>(
    const std::string& code, unsigned char islet, const std::string& var_name, std::size_t line, std::size_t column)
    {
        std::string error;
        error+=constructCodePosition(code, line, column, 1);
        error+="\n";

        error.append(red_tag);
        error.append(this->prefix);
        error.append(" has found an incorrect type input.\n");
        error.append("\nThis is because you have provided no type for the declaration of a let/const variable.");
        error.append("\nWhen there is a let/const declaration, you must provide either a value or a type that can be used to infer the type of the variable");
        error.append("\n\n");
        error.append(this->prefix);
        error.append(" will show you a correct example of how to do this.\n");
        error.append(reset_tag);
        error.append("\n");
        error.append(green_tag);

        error.append(std::to_string(line+1));
        error.append(" | ");
        if(islet)   error.append("let ");
        else   error.append("const ");
        
        error.append(var_name);
        error.append(bold_tag);
        error.append(" : type = value;");
        error.append(reset_tag);
        error.append(bold_tag);

        error.append("\n\n");
        error.append(reset_tag);
        error.append(magenta_tag);
        error.append("[Tip]: ");
        error.append("You can provide either a type or a value, if you provide the value ");
        error.append(this->prefix);
        error.append(" will automatically infer the variable's type!\n");
        error.append(reset_tag); 

        errors.push_back(error);
    }

    void ErrorBuilder::showErrors()
    {
        for(auto& error : this->errors)
        {
            std::cout << error << std::endl;
            std::cout << "----------------------------------------------------" << std::endl;
        }
    }

}
}