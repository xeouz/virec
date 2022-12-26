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
enum class EType
{
    Void,
    Char,
    Short,
    Int,
    Long,
    Float,
    Double,
    Bool,
    Array,
    Custom,
    Any,
};

// Type Map
inline std::unordered_map<std::string, EType> type_map=
{
    {"void", EType::Void},
    {"char", EType::Char},
    {"short", EType::Short},
    {"int", EType::Int},
    {"long", EType::Long},
    {"float", EType::Float},
    {"double", EType::Double},
    {"bool", EType::Bool},
    {"any", EType::Any}
};
inline std::unordered_map<EType, std::string> typestr_map=
{
    {EType::Void, "void"},
    {EType::Char, "char"},
    {EType::Short, "short"},
    {EType::Int, "int"},
    {EType::Long, "long"},
    {EType::Float, "float"},
    {EType::Double, "double"},
    {EType::Bool, "bool"},
    {EType::Custom, "custom"},
    {EType::Any, "any"},
};
inline std::unordered_map<std::string, int> custom_type_sizes=
{ };

// Prototypes
inline std::string getMapFromType(EType const& type);
inline EType getTypeFromMap(std::string typestr);

// Classes
class Base
{
protected:
    EType type;
    int8_t size;
public:
    int8_t precedence;
    bool is_const;
    bool is_signed;
    
    Base(bool _is_const=true)
    {
        type = EType::Void;
        size = 0;
        is_const = _is_const;
        is_signed = true;
    }

    virtual ~Base()=default;
    virtual EType const& getType() const { return type; }
    virtual int8_t getSize() const { return size; }

    virtual unsigned int getDepth() const { return 0; }

    virtual void setSize(unsigned int new_size)
    {
        size=new_size;
    }
    
    virtual bool isSame(Base* const other) const
    {
        return (getType() == other->getType()); 
    }
    virtual bool isSame(std::string const& other) const
    {
        return (getType() == getTypeFromMap(other)); 
    }
};

inline bool isSame(Base* const a, Base* const b);
inline bool isSame(Base* const a, const char*  b);

inline std::unique_ptr<Base> construct(std::string typestr, bool create_custom=false);
inline std::unique_ptr<Base> construct(EType const& type);
inline Base* getArrayRootType(Base* const type);

inline std::ostream& operator<<(std::ostream& os, Base const& type)
{
    os << getMapFromType(type.getType());
    return os;
}

class Void : public Base
{
    std::string name;
public:
    Void(std::string name="", bool _is_const=true)
    {
        type = EType::Void;
        size = 0;
        precedence = 0;
        is_const=_is_const;
        this->name = name;
    }

    std::string const& getName() const
    {
        return name;
    }
    void setName(std::string const& _name)
    {
        name=_name;
    }
};

class Char : public Base
{
public:
    Char(bool _is_const=true)
    {
        type = EType::Char;
        size = 1;
        precedence = 1;
        is_const=_is_const;
    }
};

class Short : public Base
{
public:
    Short(bool _is_const=true)
    {
        type = EType::Short;
        size = 2;
        precedence = 2;
        is_const=_is_const;
    }
};

class Int : public Base
{
public:
    Int(bool _is_const=true)
    {
        type = EType::Int;
        size = 4; 
        precedence = 3;
        is_const=_is_const;
    }
};

class Long : public Base
{
public:
    Long(bool _is_const=true)
    {
        type = EType::Long;
        size = 8;
        precedence = 4;
        is_const=_is_const;
    }
};

class Float : public Base
{
public:
    Float(bool _is_const=true)
    {
        type = EType::Float;
        size = 4; 
        precedence = 5;
        is_const=_is_const;
    }
};

class Double : public Base
{
public:
    Double(bool _is_const=true)
    {
        type = EType::Double;
        size = 8;
        precedence = 8;
        is_const=_is_const;
    }
};

class Bool : public Base
{
public:
    Bool(bool _is_const=true)
    {
        type = EType::Bool;
        size = 1;
        is_const=_is_const;
        is_signed=false;
    }
};

class Array : public Base
{
    std::unique_ptr<Base> child;
    unsigned int length;
public:
    Array(std::unique_ptr<Base> b, int length, bool _is_const=true)
    {
        this->type = EType::Array;
        this->child = std::move(b);
        this->length = length;
        this->size = child->getSize() * length;
        is_const=_is_const;
    }
    Array(Base* b, int length, bool _is_const=true)
    {
        this->type = EType::Array;
        this->child = std::unique_ptr<Base>(b);
        this->length = length;
        this->size = child->getSize() * length;
        is_const=_is_const;
    }

    Base* getChild() const 
    {
        return child.get(); 
    }
    EType getChildType() const
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

    bool isSame(Base* const other)
    {
        if(other->getType() == EType::Array)
        {
            Array* other_array = static_cast<Array*>(other);

            bool same_child = types::isSame(child.get(), other_array->getChild());
            bool same_length = (length == other_array->getLength());
       
            bool b=false;

            bool other_has_auto=getArrayRootType(other_array)->getType()==EType::Void;
            bool this_has_auto=false;
            if(child->getType()==EType::Array)
            {
                this_has_auto=getArrayRootType(static_cast<Array*>(child.get()))->getType()==EType::Void;
            }
            else
            {
                this_has_auto=child->getType()==EType::Void;
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
    Custom(std::string name, long size, bool _is_const=true)
    : name(name)
    {
        this->type=EType::Custom;
        this->size=size;
        this->is_const=_is_const;
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

class Any : public Base
{
public:
    Any(bool _is_const=true)
    {
        this->type=EType::Any;
        this->size=0;
        this->is_const=_is_const;
    }
};

// Functions
inline bool isSame(Base* const a, Base* const b)
{
    if(a->getType() == b->getType())
    {
        if(a->getType() != EType::Array)
        {
            if(a->getType() == EType::Custom)
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
inline bool isSame(Base* const a, const char* b)
{
    auto t=construct(b);
    bool same=isSame(a, t.get());
    return same;
}

inline std::unique_ptr<Base> copyType(Base* const type)
{
    if(type->getType() == EType::Array)
    {
        Array* array = static_cast<Array*>(type);
        return std::make_unique<Array>(copyType(array->getChild()), array->getLength());
    }
    else if(type->getType() == EType::Custom)
    {
        auto* ctype=(Custom*)type;
        return construct(ctype->getName(), true);
    }
    else if(type->getType() == EType::Void)
    {
        auto* vtype=(Void*)type;
        return std::make_unique<types::Void>(vtype->getName());
    }
    else
    {
        return construct(type->getType());
    }
}

inline void printAsArray(Base* const type)
{
    auto* a=static_cast<Array*>(type);
    std::cout << *a->getChild() << "[" << a->getLength() << "]" << std::endl;
}
inline Base* getArrayRootType(Base* const arr)
{
    Base* t;

    auto* a=static_cast<Array*>(arr);
    t=a->getChild();

    while(t->getType() == EType::Array)
    {
        a=static_cast<Array*>(t);
        t=a->getChild();
    }
    return t;
}

inline EType getTypeFromMap(std::string typestr)
{
    if(type_map.contains(typestr))
    {
        return type_map[typestr];
    }
    else
    {
        return EType::Custom;
    }
}
inline std::string getMapFromType(EType const& type)
{
    if(type==EType::Array)
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
    EType type=getTypeFromMap(typestr);
    switch(type)
    {
        case EType::Void:
            return std::make_unique<Void>();
        case EType::Char:
            return std::make_unique<Char>();
        case EType::Short:
            return std::make_unique<Short>();
        case EType::Int:
            return std::make_unique<Int>();
        case EType::Long:
            return std::make_unique<Long>();
        case EType::Float:
            return std::make_unique<Float>();
        case EType::Double:
            return std::make_unique<Double>();
        case EType::Bool:
            return std::make_unique<Bool>();
        case EType::Custom:
        {
            if(!create_custom)
            {
                // std::cout << "Typestr: " << typestr << std::endl;
                return std::make_unique<Void>(typestr);
            }
            
            return std::make_unique<Custom>(typestr, custom_type_sizes.at(typestr));
        }
        case EType::Any:
            return std::make_unique<Any>();

        default:
            return std::make_unique<Void>();
    }
}
inline std::unique_ptr<Base> construct(EType const& type)
{
    switch(type)
    {
        case EType::Void:
            return std::make_unique<Void>();
        case EType::Char:
            return std::make_unique<Char>();
        case EType::Short:
            return std::make_unique<Short>();
        case EType::Int:
            return std::make_unique<Int>();
        case EType::Long:
            return std::make_unique<Long>();
        case EType::Float:
            return std::make_unique<Float>();
        case EType::Double:
            return std::make_unique<Double>();
        case EType::Bool:
            return std::make_unique<Bool>();
        
        default:
            return std::make_unique<Void>();
    }
}

inline bool isUserDefined(EType type)
{
    return (type == EType::Custom);
}
inline bool isUserDefined(Base* _type)
{
    EType type=_type->getType();
    return isUserDefined(type);
}
inline bool isTypeFloatingPoint(EType type)
{
    return (type==EType::Double) || (type==EType::Float);
}
inline bool isTypeFloatingPoint(Base* type)
{
    return isTypeFloatingPoint(type->getType());
}

inline void addTypeToMap(std::string name)
{
    type_map.insert(std::make_pair(name,EType::Custom));
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

inline bool isNumericType(EType type)
{
    if(type==EType::Int
    || type==EType::Char
    || type==EType::Float
    || type==EType::Double)
    {
        return true;
    }
    return false;
}
inline bool isNumericType(Base* type)
{
    return isNumericType(type->getType());
}

} // namespace types
} // namespace vire