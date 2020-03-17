#include "ash/parser/keywords.h"

using namespace ash::parser;

KeywordMap::KeywordMap() {}

std::optional<Kw> KeywordMap::operator[](std::string_view str) const {
  using namespace std::string_view_literals;

  auto const len = str.length();
  if (len < 2 || len > 8) {
    return std::nullopt;
  }
  auto const rest = str.substr(1);

  switch (str[0]) {
    default:
      break;

    case 'a':
      if (len == 2 && str[1] == 's') {
        return Kw::As;
      }
      if (len == 3 && str[1] == 'n' && str[2] == 'd') {
        return Kw::And;
      }
      break;

    case 'b':
      if (rest == "reak"sv) {
        return Kw::Break;
      }
      break;

    case 'c':
      if (rest == "ontinue"sv) {
        return Kw::Continue;
      }
      break;

    case 'd':
      if (len == 2 && str[1] == 'o') {
        return Kw::Do;
      }
      if (len == 3 && str[1] == 'y' && str[2] == 'n') {
        return Kw::Dyn;
      }
      break;

    case 'e':
      if (rest == "lse"sv) {
        return Kw::Else;
      }
      if (rest == "nv"sv) {
        return Kw::Env;
      }
      if (rest == "rror"sv) {
        return Kw::Error;
      }
      if (rest == "xit"sv) {
        return Kw::Exit;
      }
      break;

    case 'f':
      if (rest == "ail"sv) {
        return Kw::Fail;
      }
      if (rest == "or"sv) {
        return Kw::For;
      }
      if (rest == "un"sv) {
        return Kw::Fun;
      }
      break;

    case 'i':
      if (len == 2) {
        switch (str[1]) {
          case 'f':
            return Kw::If;
          case 'n':
            return Kw::In;
          case 's':
            return Kw::Is;
          default:
            break;
        }
      }
      break;

    case 'l':
      if (rest == "et"sv) {
        return Kw::Let;
      }
      if (rest == "oop"sv) {
        return Kw::Loop;
      }
      break;

    case 'm':
      switch (len) {
        case 2:
          if (str[1] == 'y') {
            return Kw::My;
          }
          break;

        case 3:
          if (str[1] == 'u' && str[2] == 't') {
            return Kw::Mut;
          }
          break;

        case 4:
          if (str[1] == 'e' && str[2] == 't' && str[3] == 'a') {
            return Kw::Meta;
          }
          break;

        case 5:
          if (str[1] == 'a' && str[2] == 't' && str[3] == 'c' &&
              str[4] == 'h') {
            return Kw::Match;
          }
          break;

        default:
          break;
      }
      break;

    case 'n':
      if (len == 3 && str[1] == 'o' && str[2] == 't') {
        return Kw::Not;
      }
      break;

    case 'o':
      if (len == 2 && str[1] == 'r') {
        return Kw::Or;
      }
      break;

    case 'r':
      if (rest == "eturn"sv) {
        return Kw::Return;
      }
      break;

    case 't':
      if (rest == "his"sv) {
        return Kw::This;
      }
      if (rest == "rait"sv) {
        return Kw::Trait;
      }
      if (rest == "ype"sv) {
        return Kw::Type;
      }
      break;

    case 'v':
      if (len == 3 && str[1] == 'a' && str[2] == 'r') {
        return Kw::Var;
      }
      break;

    case 'w':
      if (rest == "hile"sv) {
        return Kw::While;
      }
      break;
  }

  return std::nullopt;
}
