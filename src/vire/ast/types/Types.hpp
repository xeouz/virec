#pragma once

#include <string>

namespace vire
{

enum class types
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

class BaseType
{
protected:
    std::string value;
    types type;
    
public:
    virtual ~BaseType() = default;

    virtual std::string getValue() const
    { return value; }
    virtual types getType() const
    { return type; }

    virtual bool isSame(const BaseType& other) const
    { return (getType() == other.getType()); }
};

class CharType : public BaseType
{
public:
    CharType(std::string value);
};

class IntType : public BaseType
{
public:
    IntType(std::string value);
};

class FloatType : public BaseType
{
public:
    FloatType(std::string value);
};

class DoubleType : public BaseType
{
public:
    DoubleType(std::string value);
};

class BoolType : public BaseType
{
public:
    BoolType(std::string value);
};

class ArrayType : public BaseType
{
    BaseType* child;
public:
    ArrayType(std::string value, BaseType* child);

    types getChildType() const;
};

} // namespace vire