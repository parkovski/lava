#ifndef ASH_PARSER_KEYWORDS_H_
#define ASH_PARSER_KEYWORDS_H_

#include <string_view>
#include <optional>

namespace ash::parser {

enum class Kw {
  And,
  As,
  Auth,
  Break,
  Continue,
  Do,
  Dyn,
  Else,
  Env,
  Error,
  Exit,
  Fail,
  For,
  Fun,
  If,
  In,
  Is,
  Let,
  Loop,
  Match,
  Meta,
  Mut,
  My,
  Not,
  Or,
  Return,
  This,
  Trait,
  Type,
  Var,
  While,
};

class KeywordMap {
public:
  explicit KeywordMap();

  std::optional<Kw> operator[](std::string_view str) const;
};

} // namespace ash::parser

#endif // ASH_PARSER_KEYWORDS_H_
