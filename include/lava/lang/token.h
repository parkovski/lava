#ifndef LAVA_LANG_TOKEN_H_
#define LAVA_LANG_TOKEN_H_

#include "sourcedoc.h"
#include <string_view>

namespace lava::lang {

enum {
  TkInvalid = -1,
  TkEof = 0,

  TkWhitespace,
  TkLineComment,
  TkBlockComment,

  TkIntLiteral,
  TkHexLiteral,
  TkBinLiteral,
  TkFloatLiteral,
  TkDoubleLiteral,
  TkStringLiteral,

  TkIdent,

  TkTilde,
  TkExcl,
  TkExclEq,
  TkPercent,
  TkPercentEq,
  TkHat,
  TkHatEq,
  TkAnd,
  TkAndAnd,
  TkAndEq,
  TkStar,
  TkStarStar,
  TkStarStarEq,
  TkStarEq,
  TkLeftParen,
  TkRightParen,
  TkMinus,
  TkMinusMinus,
  TkMinusEq,
  TkMinusRightArrow,
  TkPlus,
  TkPlusPlus,
  TkPlusEq,
  TkEq,
  TkEqEq,
  TkEqRightArrow,
  TkOr,
  TkOrOr,
  TkOrEq,
  TkLeftSquareBracket,
  TkRightSquareBracket,
  TkLeftBrace,
  TkRightBrace,
  TkSemi,
  TkColon,
  TkColonColon,
  TkLess,
  TkLessLess,
  TkLessLessEq,
  TkLessEq,
  TkLessMinusLess,
  TkLessMinusLessEq,
  TkGreater,
  TkGreaterGreater,
  TkGreaterGreaterEq,
  TkGreaterEq,
  TkGreaterMinusGreater,
  TkGreaterMinusGreaterEq,
  TkComma,
  TkDot,
  TkDotDot,
  TkDotDotDot,
  TkSlash,
  TkSlashEq,
  TkQuestion,

  TkFun,
  TkIf,
  TkElse,
};

struct Token {
  const SourceDoc *doc;
  uint32_t loc;
  int what;

  std::string_view text() const {
    std::string_view content{doc->content()};
    size_t off0 = doc->get_loc(loc).offset;
    size_t off1 = doc->get_loc(loc + 1).offset;
    return content.substr(off0, off1 - off0);
  }

  uint32_t line() const {
    return doc->get_loc(loc).line;
  }

  uint32_t column() const {
    return doc->get_loc(loc).column;
  }
};

} // namespace lava::lang

#endif // LAVA_LANG_TOKEN_H_
