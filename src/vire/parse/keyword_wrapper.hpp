#ifndef KEYWORD_HASH_CODE
#define KEYWORD_HASH_CODE

#include "vire/lex/token.hpp"

namespace vire
{

struct KeywordTokenCode
{
    enum
    {
        kw_func=tok_func,
        kw_and=tok_and,
        kw_or=tok_or,
        kw_if=tok_if,
        kw_else=tok_else,
        kw_var=tok_var,
        kw_let=tok_let,
        kw_const=tok_const,
        kw_true=tok_true,
        kw_false=tok_false,
        kw_as=tok_as,
        kw_new=tok_new,
        kw_delete=tok_delete,
        kw_class=tok_class,
        kw_union=tok_union,
        kw_struct=tok_struct,
        kw_extern=tok_extern,
        kw_for=tok_for,
        kw_while=tok_while,
        kw_return=tok_return,
        kw_break=tok_break,
        kw_continue=tok_continue,
        kw_returns=tok_returns,
        kw_proto=tok_proto,
        kw_extends=tok_extends,
        kw_try=tok_try,
        kw_except=tok_except,
        kw_unsafe=tok_unsafe,
        kw_constructor=tok_constructor,
    };
};

}
#endif