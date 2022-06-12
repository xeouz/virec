#pragma once

#include <vector>
#include <string>

namespace vire
{
namespace errors
{

enum errortypes
{
    lex_unknown_char,

    parse_unexpected_token,
    parse_unexpected_eof,

    analyze_requires_type,
};

class ErrorBuilder
{
    std::vector<std::string> errors;
    std::string prefix;
public:
    
    ErrorBuilder() : prefix("This program") {};
    ErrorBuilder(const std::string& prefix) : prefix(prefix) {};

    void setPrefix(const std::string& newprefix) {prefix=newprefix;}

    std::string constructCodePosition
    (const std::string& input, std::size_t line, std::size_t column, int column_len=1);

    template<errortypes X>
    void addError();

    template<errortypes X>
    void addError(const std::string& code, char _char, char fix='\0', std::size_t line=0, std::size_t column=0); // <errortypes::lex_unknown_char>



    template<errortypes X>
    void addError(const std::string& code, unsigned char islet, const std::string& varname="my_var", std::size_t line=0, std::size_t column=0); // <errortypes::analyzer_requires_type>

    void showErrors();
};

}
}