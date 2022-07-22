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
std::unordered_map<std::string, TypeNames> type_map=
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
    Void();
};

class Char : public Base
{
public:
    Char();
};

class Int : public Base
{
public:
    Int();
};

class Float : public Base
{
public:
    Float();
};

class Double : public Base
{
public:
    Double();
};

class Bool : public Base
{
public:
    Bool();
};

class Array : public Base
{
    Base* child;
public:
    Array(Base*);

    TypeNames getChildType() const;
};

// Functions
TypeNames getTypeFromMap(std::string typestr);
Base* construct(std::string);

} // namespace types
} // namespace vire