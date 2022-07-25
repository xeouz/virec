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
inline std::unordered_map<TypeNames, std::string> typestr_map=
{
    {TypeNames::Void, "void"},
    {TypeNames::Char, "char"},
    {TypeNames::Int, "int"},
    {TypeNames::Float, "float"},
    {TypeNames::Double, "double"},
    {TypeNames::Bool, "bool"},
};

// Prototypes
inline std::string getMapFromType(TypeNames type);
inline TypeNames getTypeFromMap(std::string typestr);

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
    virtual bool isSame(std::string const& other) const
    { return (getType() == getTypeFromMap(other)); }

    friend std::ostream& operator<<(std::ostream& os, Base const& type);
};

inline bool operator==(const Base& lhs, const Base& rhs)
{ return lhs.isSame(rhs); }
inline bool operator!=(const Base& lhs, const Base& rhs)
{ return !(lhs == rhs); }

inline bool operator==(const Base& lhs, const char* rhs)
{ return lhs.isSame(rhs); }
inline bool operator!=(const Base& lhs, const char* rhs)
{ return !(lhs == rhs); }

inline std::ostream& operator<<(std::ostream& os, Base const& type)
{
    os << getMapFromType(type.getType());
    return os;
}

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

inline std::string getMapFromType(TypeNames type)
{
    if(typestr_map.contains(type))
    {
        return typestr_map[type];
    }
    else
    {
        return "";
    }
}

inline Base construct(std::string typestr)
{
    TypeNames type=getTypeFromMap(typestr);
    switch(type)
    {
        case TypeNames::Char:
            return Char();
        case TypeNames::Int:
            return Int();
        case TypeNames::Float:
            return Float();
        case TypeNames::Double:
            return Double();
        case TypeNames::Bool:
            return Bool();
        case TypeNames::Void:
            return Void();

        default:
            return Base();
    }
}

} // namespace types
} // namespace vire