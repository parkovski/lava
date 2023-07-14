#ifndef LAVA_SYN_LEXER_H_
#define LAVA_SYN_LEXER_H_

#include <cstdint>
#include <cstddef>
#include <string>
#include <string_view>

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
};

struct Token {
  const SourceDoc *doc;
  SourceLoc start;
  SourceLoc end;
  int what;
};

struct Lexer {
private:
  const SourceDoc *doc;
  std::string_view text;
  SourceLoc loc;

public:
  explicit Lexer(const SourceDoc &doc) noexcept;

  Token lex();

private:
  int getch(unsigned lookahead = 0) const;
  void nextch();

  void lex_whitespace(Token &token);
  void lex_line_comment(Token &token);
  void lex_block_comment(Token &token);
  void lex_number(Token &token);
  void lex_hex_number(Token &token);
  void lex_binary_number(Token &token);
  void lex_decimal_part(Token &token);
  void lex_string(Token &token);
  void lex_ident(Token &token);
  void lex_symbol_or_invalid(Token &token);
};

} // namespace lava::syn

#endif // LAVA_SYN_LEXER_H_
