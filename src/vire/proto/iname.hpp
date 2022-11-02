#pragma once

#include <string>

namespace vire
{
namespace proto
{

class IName
{
    std::string prefixed_name;

    void refresh();
protected:
    std::string name;
    std::string prefix;
public:
    IName(std::string prefix, std::string name);

    void setName(const char* name);
    void setName(std::string const& name);

    void setPrefix(const char* prefix);
    void setPrefix(std::string const& prefix);

    std::string const& get();
};

}
}