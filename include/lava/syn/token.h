#ifndef LAVA_SYN_TOKEN_H_
#define LAVA_SYN_TOKEN_H_

#include <string>
#include <string_view>
#include <cstdint>

namespace lava::syn {

struct SourceDoc {
  std::string name;
  std::string content;
};

struct SourceLoc {
  size_t offset;
  uint32_t line;
  uint32_t column;

  SourceLoc() noexcept
    : offset{0}
    , line{1}
    , column{1}
  {}
};

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
  SourceLoc start;
  SourceLoc end;
  int what;

  std::string_view text() const {
    return std::string_view(doc->content)
      .substr(start.offset, end.offset - start.offset);
  }
};


} // namespace lava::syn

#endif // LAVA_SYN_TOKEN_H_
