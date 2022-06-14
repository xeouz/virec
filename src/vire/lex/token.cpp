#pragma once

#include <iostream>
#include <string>

#include "token.hpp"

namespace vire
{
class Viretoken
{
public:
    int type;
    std::string value;
    char invalid;

    std::size_t line;
    std::size_t charpos;

    Viretoken(std::string value, int type) 
    : value(value), type(type), line(0), charpos(0)
    {
        if(type>=0)
        {
            this->invalid=1;
        }
    }
    Viretoken(std::string value, int type, std::size_t line, std::size_t charpos) 
    : value(value), type(type), line(line), charpos(charpos)
    {
        if(type>=0)
        {
            this->invalid=1;
        }
    }

    ~Viretoken(){}

    inline friend std::ostream& operator<<(std::ostream& os, const Viretoken& tok);
};

inline std::ostream& operator<<(std::ostream& os, const vire::Viretoken& tok)
{
    os<<tokToStr(tok.type);
    return os;
}

static const char* tokToStr(int tok)
{
    switch(tok)
    {
        case tok_eof: return "tok_eof";

        case tok_func: return "tok_func";
        case tok_var: return "tok_var";
        case tok_vardef: return "tok_vardef";

        case tok_id: return "tok_id";
        case tok_int: return "tok_int";
        case tok_float: return "tok_float";
        case tok_double: return "tok_double";
        
        case tok_char: return "tok_char";
        case tok_str: return "tok_str";

        case tok_lbrace: return "tok_lbrace";
        case tok_lbrack: return "tok_lbrack";
        case tok_lparen: return "tok_lparen";
        case tok_rbrace: return "tok_rbrace";
        case tok_rbrack: return "tok_rbrack";
        case tok_rparen: return "tok_rparen";

        case tok_semicol: return "tok_semicol";

        case tok_equal: return "tok_equal";
        case tok_dequal: return "tok_dequal";

        case tok_plus: return "tok_plus";
        case tok_minus: return "tok_minus";
        case tok_mul: return "tok_mul";
        case tok_div: return "tok_div";
        case tok_mod: return "tok_mod";
        
        case tok_incr: return "tok_incr";
        case tok_decr: return "tok_decr";

        case tok_or: return "tok_or";
        case tok_and: return "tok_and";

        case tok_colon: return "tok_colon";
        case tok_comma: return "tok_comma";

        case tok_const: return "tok_const";
        case tok_extern: return "tok_extern";

        case tok_for: return "tok_for";
        case tok_while: return "tok_while";

        case tok_lessthan: return "tok_lessthan";
        case tok_morethan: return "tok_morethan";
        case tok_lesseq: return "tok_lesseq";
        case tok_moreeq: return "tok_moreeq";

        case tok_rarrow: return "tok_rarrow";
        case tok_returns: return "tok_returns";

        case tok_proto: return "tok_proto";
        case tok_class: return "tok_class";
        case tok_extends: return "tok_extends";

        case tok_union: return "tok_union";
        case tok_struct: return "tok_struct";

        case tok_return: return "tok_return";
        case tok_break: return "tok_break";
        case tok_continue: return "tok_continue";

        case tok_if: return "tok_if";
        case tok_else: return "tok_else";

        case tok_new: return "tok_new";
        case tok_delete: return "tok_delete";
        case tok_unsafe: return "tok_unsafe";
        
        case tok_let: return "tok_let";
        case tok_reference: return "tok_reference";

        case tok_try: return "tok_try";
        case tok_catch: return "tok_catch";

        case tok_dot: return "tok_dot";

        default: return "unknown";
    }
}

}