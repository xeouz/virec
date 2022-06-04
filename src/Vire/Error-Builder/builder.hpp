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
};

class ErrorBuilder
{
    std::vector<std::string> errors;
    std::string prefix;
public:
    
    ErrorBuilder() : prefix("This program") {};
    ErrorBuilder(const std::string& prefix) : prefix(prefix) {};

    void setPrefix(const std::string& newprefix) {prefix=newprefix;}

    template<errortypes X>
    void addError();

    template<errortypes X>
    void addError(char _char, char fix='\0'); // <errortypes::lex_unknown_char>

    void showErrors();
};

}
}