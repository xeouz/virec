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
public:
    std::string name;
    std::string prefix;
public:
    IName();
    IName(std::string name, std::string prefix="_");

    void setName(const char* name);
    void setName(std::string const& name);

    void setPrefix(const char* prefix);
    void setPrefix(std::string const& prefix);

    std::string const& get() const;

    bool isSame(IName const& rhs) const;
    bool operator==(IName const& rhs) const;
};

}
}

namespace std {

  template <>
  struct hash<vire::proto::IName>
  {
    std::size_t operator()(const vire::proto::IName& k) const
    {
      using std::size_t;
      using std::hash;
      using std::string;

      return hash<string>()(k.get());
    }
  };

}