#pragma once

#include <unordered_map>
#include <string>
#include <iostream>
#include <memory>

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
    int8_t size;
public:
    Base()
    {
        type = TypeNames::Void;
        size = 0;
    }

    virtual ~Base() {}

    virtual TypeNames getType() const { return type; }
    virtual int8_t getSize() const { return size; }
    
    virtual bool isSame(Base* const& other) const
    {
        return (getType() == other->getType()); 
    }
    virtual bool isSame(std::string const& other) const
    {
        return (getType() == getTypeFromMap(other)); 
    }
};

inline bool isSame(Base* const& a, Base* const& b);
inline bool isSame(Base* const& a, const char*  b);

inline std::unique_ptr<Base> construct(std::string typestr);
inline Base* getArrayRootType(Base* const& type);

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
        size = 0;
    }
};

class Char : public Base
{
public:
    Char()
    {
        type = TypeNames::Char;
        size = 1;
    }
};

class Int : public Base
{
public:
    Int()
    {
        type = TypeNames::Int;
        size = 4;
    }
};

class Float : public Base
{
public:
    Float()
    {
        type = TypeNames::Float;
        size = 4;
    }
};

class Double : public Base
{
public:
    Double()
    {
        type = TypeNames::Double;
        size = 8;
    }
};

class Bool : public Base
{
public:
    Bool()
    {
        type = TypeNames::Bool;
        size = 1;
    }
};

class Array : public Base
{
    std::unique_ptr<Base> child;
    unsigned int length;
public:
    Array(std::unique_ptr<Base> b, int length)
    {
        this->type = TypeNames::Array;
        this->child = std::move(b);
        this->length = length;
        this->size = child->getSize() * length;
    }

    Base* getChild() const 
    {
        return child.get(); 
    }
    TypeNames getChildType() const
    {
        return child->getType();
    }

    void setChild(std::unique_ptr<Base> new_child)
    {
        child.release();
        child = std::move(new_child);
    }

    unsigned int getLength() const
    {
        return length;
    }

    bool isSame(Base* const& other)
    {
        if(other->getType() == TypeNames::Array)
        {
            Array* other_array = static_cast<Array*>(other);
            bool same_child = types::isSame(child.get(), other_array->getChild());
            
            bool same_length = (length == other_array->getLength());
            
            bool b=false;

            bool other_has_auto=getArrayRootType(other_array)->getType()==TypeNames::Void;
            bool this_has_auto=false;
            if(child->getType()==TypeNames::Array)
            {
                this_has_auto=getArrayRootType(static_cast<Array*>(child.get()))->getType()==TypeNames::Void;
            }
            else
            {
                this_has_auto=child->getType()==TypeNames::Void;
            }

            if(this_has_auto || other_has_auto)
            {
                b = same_length;
            }
            else
            {
                b = same_child && same_length;
            }

            return b;
        }
        return false;
    }
};

// Functions
inline bool isSame(Base* const& a, Base* const& b)
{
    if(a->getType() == b->getType())
    {
        if(a->getType() != TypeNames::Array)
        {
            return true;
        }
        else
        {
            Array* a_array = static_cast<Array*>(a);
            bool f=a_array->isSame(b);

            return f;
        }
    }
    return false;
}
inline bool isSame(Base* const& a, const char* b)
{
    auto t=construct(b);
    bool same=isSame(a, t.get());
    t.release();
    return same;
}

inline void printAsArray(Base* const& type)
{
    auto* a= static_cast<Array*>(type);
    std::cout << a->getChild() << "[" << a->getLength() << "]";
}

inline Base* getArrayRootType(Base* const& arr)
{
    Base* t;

    auto* a=static_cast<Array*>(arr);
    t=a->getChild();

    while(t->getType() == TypeNames::Array)
    {
        a=static_cast<Array*>(t);
        t=a->getChild();
    }
    return t;
}

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
    if(type==TypeNames::Array)
    {
        return "array";
    }
    else if(typestr_map.contains(type))
    {
        return typestr_map[type];
    }
    else
    {

        return "undefined";
    }
}

inline std::unique_ptr<Base> construct(std::string typestr)
{
    TypeNames type=getTypeFromMap(typestr);
    switch(type)
    {
        case TypeNames::Void:
            return std::make_unique<Void>();
        case TypeNames::Char:
            return std::make_unique<Char>();
        case TypeNames::Int:
            return std::make_unique<Int>();
        case TypeNames::Float:
            return std::make_unique<Float>();
        case TypeNames::Double:
            return std::make_unique<Double>();
        case TypeNames::Bool:
            return std::make_unique<Bool>();
        
        default:
            return std::make_unique<Void>();
    }
}

} // namespace types
} // namespace vire