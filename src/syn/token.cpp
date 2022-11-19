#include "lava/syn/token.h"

#include <ostream>

namespace lava::syn {

std::string_view to_string(Tk tk) {
#define LAVA_TK(t) case Tk::t: return #t;
  switch (tk) {
    default: return {"(undefined)"};
    LAVA_TOKENS(LAVA_TK)
  }
#undef LAVA_TK
}

std::ostream &operator<<(std::ostream &os, Tk tk) {
  return os << to_string(tk);
}

std::string_view to_string(Kw kw) {
#define LAVA_KW(k) case Kw::k: return #k;
  switch (kw) {
    default: return {"(undefined)"};
    LAVA_KEYWORDS(LAVA_KW)
  }
#undef LAVA_KW
}

std::ostream &operator<<(std::ostream &os, Kw kw) {
  return os << to_string(kw);
}

Kw kw_from_string(std::string_view str) {
  using namespace std::string_view_literals;

  if (str.size() < 2 || str.size() > 9) {
    return Kw::_undef;
  }

  std::string_view rest = str.substr(1);
  switch (str[0]) {
  case 'a':
    if (rest == "nd"sv) return Kw::And;
    if (rest == "s"sv) return Kw::As;
    break;

  case 'b':
    if (rest == "reak"sv) return Kw::Break;
    break;

  case 'c':
    if (rest == "onst"sv) return Kw::Const;
    if (rest == "ontinue"sv) return Kw::Continue;
    break;

  case 'd':
    if (rest == "o"sv) return Kw::Do;
    break;

  case 'e':
    if (rest == "lse"sv) return Kw::Else;
    break;

  case 'f':
    if (rest == "or"sv) return Kw::For;
    if (rest == "un"sv) return Kw::Fun;
    break;

  case 'i':
    rest = rest.substr(1);
    switch (str[1]) {
    case 'f':
      if (str.length() == 2) return Kw::If;
      break;

    case 'm':
      if (rest == "plement"sv) return Kw::Implement;
      break;

    case 'n':
      if (str.length() == 2) return Kw::In;
      if (rest == "terface"sv) return Kw::Interface;
      break;

    case 's':
      if (str.length() == 2) return Kw::Is;
      break;
    }
    break;

  case 'l':
    if (rest == "oop"sv) return Kw::Loop;
    break;

  case 'm':
    if (rest == "utable"sv) return Kw::Mutable;
    break;

  case 'o':
    if (rest == "r"sv) return Kw::Or;
    break;

  case 'r':
    if (rest == "eturn"sv) return Kw::Return;
    break;

  case 's':
    if (rest == "truct"sv) return Kw::Struct;
    break;

  case 't':
    if (rest == "hen"sv) return Kw::Then;
    if (rest == "ype"sv) return Kw::Type;
    break;

  case 'w':
    if (rest == "hile"sv) return Kw::While;
    break;

  default:
    break;
  }

  return Kw::_undef;
}

} // namespace lava::syn
