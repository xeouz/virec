#pragma once

#include "type.hpp"

namespace vire
{
namespace types
{

// Class for `char`
Char::Char()
{
    this->type = TypeNames::Char;
}

// Class for `int`
Int::Int()
{
    this->type = TypeNames::Int;
}

// Class for `float`
Float::Float()
{
    this->type = TypeNames::Float;
}

// Class for `double`
Double::Double()
{
    this->type = TypeNames::Double;
}

// Class for `bool`
Bool::Bool()
{
    this->type = TypeNames::Bool;
}

// Class for arrays
Array::Array(Base* child)
{
    this->type = TypeNames::Array;
    this->child = child;
}
TypeNames Array::getChildType() const
{
    return child->getType();
}

// Functions
TypeNames getTypeFromMap(std::string typestr)
{
    if(type_map.contains(typestr))
    {
        return type_map[typestr];
    }
    else
    {
        return TypeNames::Custom;
    }
}
Base* construct(std::string typestr)
{
    TypeNames type=getTypeFromMap(typestr);
    switch(type)
    {
        case TypeNames::Char:
            return new Char();
        case TypeNames::Int:
            return new Int();
        case TypeNames::Float:
            return new Float();
        case TypeNames::Double:
            return new Double();
        case TypeNames::Bool:
            return new Bool();
        case TypeNames::Void:
            return new Void();
        
        default:
            return new Base();
    }
}

} // namespace types
} // namespace vire