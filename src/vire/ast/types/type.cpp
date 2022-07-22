#pragma once

#include <unordered_map>
#include <string>

namespace vire
{
namespace types
{
// Enum for the different types of Type-AST nodes
enum class TypeNames
{
    Void,
    Char,
    Int,
    Float,
    Double,
    Bool,
    Array,
    Custom,
};

// Type Map
inline std::unordered_map<std::string, TypeNames> type_map=
{
    {"void", TypeNames::Void},
    {"char", TypeNames::Char},
    {"int", TypeNames::Int},
    {"float", TypeNames::Float},
    {"double", TypeNames::Double},
    {"bool", TypeNames::Bool},
};

// Classes
class Base
{
protected:
    TypeNames type;
    
public:
    virtual ~Base() = default;

    virtual TypeNames getType() const
    { return type; }

    virtual bool isSame(const Base& other) const
    { return (getType() == other.getType()); }
};

class Void : public Base
{
public:
    Void()
    {
        type = TypeNames::Void;
    }
};

class Char : public Base
{
public:
    Char()
    {
        type = TypeNames::Char;
    }
};

class Int : public Base
{
public:
    Int()
    {
        type = TypeNames::Int;
    }
};

class Float : public Base
{
public:
    Float()
    {
        type = TypeNames::Float;
    }
};

class Double : public Base
{
public:
    Double()
    {
        type = TypeNames::Double;
    }
};

class Bool : public Base
{
public:
    Bool()
    {
        type = TypeNames::Bool;
    }
};

class Array : public Base
{
    Base* child;
public:
    Array(Base* b)
    {
        type = TypeNames::Array;
        child = b;
    }

    TypeNames getChildType() const
    {
        return child->getType();
    }
};

// Functions
inline TypeNames getTypeFromMap(std::string typestr)
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
inline Base* construct(std::string typestr)
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