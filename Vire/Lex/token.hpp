#pragma once

namespace vire
{
enum token
{
    tok_eof=-1,

    tok_func=-2,
    tok_var=-3,

    tok_id=-4,
    tok_int=-5,
    tok_float=-6,
    tok_double=-7,
    
    tok_char=-8,
    tok_str=-9,

    tok_lbrace=-10,
    tok_lbrack=-11,
    tok_lparen=-12,
    tok_rbrace=-13,
    tok_rbrack=-14,
    tok_rparen=-15,

    tok_semicol=-16,

    tok_equal=-17,
    tok_dequal=-18,

    tok_plus=-19,
    tok_minus=-20,
    tok_mul=-21,
    tok_div=-22,
    tok_mod=-23,
    
    tok_incr=-24,
    tok_decr=-25,

    tok_or=-26,
    tok_and=-27,

    tok_colon=-28,
    tok_comma=-29,

    tok_vardef=-30,
    tok_const=-31,
    tok_extern=-32,

    tok_for=-33,
    tok_while=-34,

    tok_lessthan=-35,
    tok_morethan=-36,
    tok_lesseq=-37,
    tok_moreeq=-38,

    tok_rarrow=-39,
    tok_returns=-40,

    tok_proto=-41,
    tok_class=-42,
    tok_extends=-43, 

    tok_union=-44,
    tok_struct=-45,

    tok_return=-46,
    tok_break=-47,
    tok_continue=-48,

    tok_if=-49,
    tok_else=-50,

    tok_new=-51,
    tok_delete=-52,
    tok_unsafe=-53,

    tok_let=-54,
    tok_reference=-55,

    tok_try=-56,
    tok_catch=-57,

    tok_dot=-58,
};

const char* tokToStr(int type);
}