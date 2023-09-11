#ifndef LAVA_LANG_TOKEN_H_
#define LAVA_LANG_TOKEN_H_

#include "sourceloc.h"

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
  LocRef loc;
  int what;
};

} // namespace lava::lang

#endif // LAVA_LANG_TOKEN_H_
