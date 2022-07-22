#pragma once

#include "Types.hpp"

namespace vire
{

CharType::CharType(std::string value)
{
    this->value = value;
    this->type = types::Char;
}

IntType::IntType(std::string value)
{
    this->value = value;
    this->type = types::Int;
}

FloatType::FloatType(std::string value)
{
    this->value = value;
    this->type = types::Float;
}

DoubleType::DoubleType(std::string value)
{
    this->value = value;
    this->type = types::Double;
}

ArrayType::ArrayType(std::string value, BaseType* child)
{
    this->value = value;
    this->type = types::Array;
    this->child = child;
}

types ArrayType::getChildType() const
{
    return child->getType();
}

}