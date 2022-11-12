#include "iname.hpp"

namespace vire
{
namespace proto
{

IName::IName()
{
    this->name="";
    this->prefix="_";
    this->prefixed_name=this->prefix+this->name;
}
IName::IName(std::string name, std::string prefix)
: name(name), prefix(prefix)
{
    this->prefixed_name=prefix+name;
}

void IName::setName(const char* name)
{
    this->name=name;
    refresh();
}
void IName::setName(std::string const& name)
{
    setName(name.c_str());
}

void IName::setPrefix(const char* prefix)
{
    this->prefix=prefix;
    refresh();
}
void IName::setPrefix(std::string const& prefix)
{
    setPrefix(prefix.c_str());
}

std::string const& IName::get() const
{
    return prefixed_name;
}

void IName::refresh()
{
    prefixed_name=prefix+name;
}

bool IName::isSame(IName const& rhs) const
{
    return this->get()==rhs.get();
}
bool IName::operator==(IName const& rhs) const
{
    return isSame(rhs);
}

}
}