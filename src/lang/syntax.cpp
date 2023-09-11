#include "lava/lang/syntax.h"

using namespace lava::lang;

Node::~Node() {}

istring Program::intern(std::string_view str) const {
  auto it = _strings.find(str);
  if (it == _strings.end()) {
    auto off0 = _the_string.size();
    auto len = str.size();
    _the_string.append(str);
    auto istr = std::string_view{_the_string}.substr(off0, len);
    _strings.emplace(istr);
    return istr;
  }
  return *it;
}
