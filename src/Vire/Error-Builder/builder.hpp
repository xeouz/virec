#pragma once

#include <vector>
#include <string>

namespace vire
{
namespace errors
{

enum class errortypes
{
    lex_unknown_char,

    parse_unexpected_token,
    parse_unexpected_eof,
};

class ErrorBuilder
{
    std::vector<std::string> errors;
public:
    ErrorBuilder(){};

    template<errortypes X>
    void addError();

    template<errortypes X>
    void addError(char _char); // <errortypes::lex_unknown_char>

    void showErrors();
};

}
}