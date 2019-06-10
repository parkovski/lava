#ifndef ASH_LEXER_H_
#define ASH_LEXER_H_

#include <string_view>
#include <iosfwd>

namespace ash {

enum class TokenKind {
  Invalid = 0,
  Tilde,
  At,
  Hash,
  Dollar,
  LParen,
  RParen,
  LBracket,
  RBracket,
  LBrace,
  RBrace,
  Backslash,
  Semi,
  Comma,
  Excl,
  ExclEq,
  Mod,
  ModEq,
  Xor,
  XorEq,
  Eq,
  EqEq,
  RArrowDbl,
  Minus,
  MinusMinus,
  MinusEq,
  RArrowSng,
  And,
  AndAnd,
  AndEq,
  Plus,
  PlusPlus,
  PlusEq,
  Star,
  StarStar,
  StarEq,
  Gt,
  Shr,
  GtEq,
  ShrEq,
  Colon,
  ColonColon,
  Dot,
  DotDot,
  DotDotDot,
  Or,
  OrOr,
  OrEq,
  RTriangle,
  Lt,
  Shl,
  LtEq,
  ShlEq,
  Spaceship,
  LTriangle,
  LArrowSng,
  Slash,
  SlashEq,

  Eof = 100,
  Whitespace,
  NewLine,
  LineComment,
  BlockComment,
  Id,
  Integer,
  Float,
  String,
};

struct Token {
  TokenKind kind;
  size_t position;

  explicit Token() noexcept
    : kind(TokenKind::Invalid), position(0)
  {}

  Token(TokenKind kind, size_t position) noexcept
    : kind(kind), position(position)
  {}
};

std::ostream &operator<<(std::ostream &, TokenKind);

class Lexer {
public:
  struct Position {
    size_t pos;
    unsigned column;
    unsigned line;
  };

  explicit Lexer() noexcept
    : Lexer("")
  {}

  explicit Lexer(std::string_view text) noexcept
    : _text(text),
      _prev{0, 1, 1},
      _cur{0, 1, 1}
  {}

  std::string_view text() const {
    return _text;
  }

  std::string_view text(size_t start, size_t end) const {
    return _text.substr(start, end - start);
  }

  void setText(std::string_view text) {
    _text = text;
    _prev = {0, 1, 1};
    _cur = {0, 1, 1};
  }

  std::string_view currentText() const {
    return text(_prev.pos, _cur.pos);
  }

  const Position &previousPosition() const {
    return _prev;
  }

  const Position &currentPosition() const {
    return _cur;
  }

  Position &currentPosition() {
    return _cur;
  }

  Token lex();

private:
  int current() const;
  int next();
  void newLine();

  TokenKind lexWhitespace();
  TokenKind lexNewLine();
  TokenKind lexId();
  TokenKind lexNumber();
  TokenKind lexString();
  TokenKind lexLineComment();
  TokenKind lexBlockComment();
  TokenKind lexOperator();

  std::string_view _text;
  Position _prev;
  Position _cur;
};

} // namespace ash

#endif /* ASH_LEXER_H_ */
