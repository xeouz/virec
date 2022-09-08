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
    {TypeNames::Custom, "custom"},
};
inline std::unordered_map<std::string, int> custom_type_sizes=
{

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
    int8_t precedence;
    
    Base()
    {
        type = TypeNames::Void;
        size = 0;
    }

    virtual ~Base()=default;
    virtual TypeNames getType() const { return type; }
    virtual int8_t getSize() const { return size; }

    virtual unsigned int getDepth() const { return 0; }
    
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

inline std::unique_ptr<Base> construct(std::string typestr, bool create_custom=false);
inline std::unique_ptr<Base> construct(TypeNames type);
inline Base* getArrayRootType(Base* const& type);

inline std::ostream& operator<<(std::ostream& os, Base const& type)
{
    os << getMapFromType(type.getType());
    return os;
}

class Void : public Base
{
    std::string name;
public:
    Void(std::string name="")
    {
        type = TypeNames::Void;
        size = 0;
        precedence = 0;
        this->name = name;
    }

    std::string const& getName() const
    {
        return name;
    }
};

class Char : public Base
{
public:
    Char()
    {
        type = TypeNames::Char;
        size = 1;
        precedence = 2;
    }
};

class Int : public Base
{
public:
    Int()
    {
        type = TypeNames::Int;
        size = 4; 
        precedence = 4;
    }
};

class Float : public Base
{
public:
    Float()
    {
        type = TypeNames::Float;
        size = 4; 
        precedence = 5;
    }
};

class Double : public Base
{
public:
    Double()
    {
        type = TypeNames::Double;
        size = 8;
        precedence = 8;
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
    Array(Base* b, int length)
    {
        this->type = TypeNames::Array;
        this->child = std::unique_ptr<Base>(b);
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

    unsigned int getDepth() const
    {
        return child->getDepth() + 1;
    }
    unsigned int getLength() const
    {
        return length;
    }
    void setLength(unsigned int new_length)
    {
        length = new_length;
        size = child->getSize() * length;
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

class Custom : public Base
{
    std::string name;
public:
    Custom(std::string name, int size)
    : name(name)
    {
        this->type=TypeNames::Custom;
        this->size=size;
    }

    std::string const& getName() const 
    {
        return name;
    }

    bool isSame(Custom* other)
    {
        return name==other->getName();
    }
};

// Functions
inline bool isSame(Base* const& a, Base* const& b)
{
    if(a->getType() == b->getType())
    {
        if(a->getType() != TypeNames::Array)
        {
            if(a->getType() == TypeNames::Custom)
            {
                return a->isSame(b);
            }

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
    return same;
}

inline std::unique_ptr<Base> copyType(Base* const& type)
{
    if(type->getType() == TypeNames::Array)
    {
        Array* array = static_cast<Array*>(type);
        return std::make_unique<Array>(copyType(array->getChild()), array->getLength());
    }
    else
    {
        return construct(type->getType());
    }
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

inline std::unique_ptr<Base> construct(std::string typestr, bool create_custom)
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
        case TypeNames::Custom:
        {
            if(!create_custom)  return std::make_unique<Void>(typestr);
            
            return std::make_unique<Custom>(typestr, custom_type_sizes.at(typestr));
        }
        
        default:
            return std::make_unique<Void>();
    }
}
inline std::unique_ptr<Base> construct(TypeNames type)
{
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

inline bool isUserDefined(TypeNames type)
{
    return (type == TypeNames::Custom) && !(type == TypeNames::Void) && !(type == TypeNames::Array);
}
inline bool isUserDefined(Base* _type)
{
    TypeNames type=_type->getType();
    return isUserDefined(type);
}
inline bool isTypeFloatingPoint(TypeNames type)
{
    return (type==TypeNames::Double) || (type==TypeNames::Float);
}
inline bool isTypeFloatingPoint(Base* type)
{
    return isTypeFloatingPoint(type->getType());
}

inline void addTypeToMap(std::string name)
{
    type_map.insert(std::make_pair(name,TypeNames::Custom));
}
inline bool isTypeinMap(std::string name)
{
    if(type_map.count(name)>0)
    {
        return true;
    }

    return false;
}

inline void addTypeSizeToMap(std::string name, unsigned int size)
{
    custom_type_sizes.insert(std::make_pair(name,size));
}

} // namespace types
} // namespace vire