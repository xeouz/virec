#pragma once

namespace vire
{
enum asttype
{
    ast_code,
    ast_var,
    ast_vardef,
    ast_varassign,

    ast_for,
    ast_while,
    
    ast_int,
    ast_float,
    ast_double,
    ast_char,
    ast_str,
    ast_bool,

    ast_call,
    ast_proto,
    ast_extern,
    ast_function,

    ast_unop,
    ast_binop,

    ast_class,

    ast_union,
    ast_struct,
    ast_type,

    ast_return,
    ast_break,
    ast_continue,

    ast_if,
    ast_ifelse,

    ast_new,
    ast_delete,
    ast_unsafe,
    
    ast_reference,

    ast_try,

    ast_array_access,
    ast_type_access,

    ast_array,

    ast_incrdecr,

    ast_cast,
};
}