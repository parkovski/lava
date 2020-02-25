#ifndef ASH_PARSER_TOKEN_H_
#define ASH_PARSER_TOKEN_H_

#include <string_view>
#include <iosfwd>

namespace ash::parser {

#define ASH_TOKENS(M)    \
  M(Invalid)             \
  M(EndOfInput)          \
  M(Whitespace)          \
  M(NewLine)             \
                         \
  M(Text)                \
  M(Ident)               \
  M(Variable)            \
  M(Path)                \
  M(Escape)              \
                         \
  M(IntLit)              \
  M(HexLit)              \
  M(BinLit)              \
  M(FloatLit)            \
                         \
  M(Backtick)            \
  M(DoubleQuote)         \
  M(SingleQuote)         \
                         \
  M(CommentLine)         \
  M(CommentShebang)      \
  M(CommentBlockStart)   \
  M(CommentBlockEnd)     \
  M(CommentKeyword)      \
                         \
  M(Tilde)               \
  M(Not)                 \
  M(NotEqual)            \
  M(At)                  \
  M(Percent)             \
  M(PercentEqual)        \
  M(Caret)               \
  M(CaretEqual)          \
  M(And)                 \
  M(AndAnd)              \
  M(AndEqual)            \
  M(Star)                \
  M(StarStar)            \
  M(StarEqual)           \
  M(StarStarEqual)       \
  M(Minus)               \
  M(MinusEqual)          \
  M(MinusArrowRight)     \
  M(Plus)                \
  M(PlusEqual)           \
  M(Equal)               \
  M(EqualEqual)          \
  M(Bar)                 \
  M(BarEqual)            \
  M(BarBar)              \
  M(Less)                \
  M(LessEqual)           \
  M(LessLess)            \
  M(LessLessEqual)       \
  M(Greater)             \
  M(GreaterEqual)        \
  M(GreaterGreater)      \
  M(GreaterGreaterEqual) \
  M(Slash)               \
  M(SlashEqual)          \
                         \
  M(LeftParen)           \
  M(RightParen)          \
  M(DollarLeftParen)     \
  M(LeftSquareBracket)   \
  M(RightSquareBracket)  \
  M(LeftBrace)           \
  M(RightBrace)          \
  M(DollarLeftBrace)     \
                         \
  M(Semicolon)           \
  M(Colon)               \
  M(ColonColon)          \
  M(Comma)               \
  M(Dot)                 \
  M(DotDot)              \
  M(DotDotDot)           \
  M(Question)            \

enum class Tk : uint16_t {
#define ASH_TK(t) t,
  ASH_TOKENS(ASH_TK)
#undef ASH_TK
};

std::string_view to_string(Tk tk);
std::ostream &operator<<(std::ostream &os, Tk tk);

struct Token {
  explicit Token(Tk id,
                 size_t index0, size_t index1,
                 unsigned line0, unsigned line1,
                 unsigned column0, unsigned column1) noexcept
    : index0{index0}, index1{index1},
      line0{line0}, line1{line1},
      column0{column0}, column1{column1},
      id{id}
  {}

  Token(const Token &) = default;
  Token &operator=(const Token &) = default;

  bool operator==(const Token &other) const {
    return index0 == other.index0
        && index1 == other.index1
        && id == other.id;
  }

  bool operator!=(const Token &other) const {
    return !(*this == other);
  }

  size_t index0;
  size_t index1;
  unsigned line0;
  unsigned line1;
  unsigned column0;
  unsigned column1;
  Tk id;
};

} // namespace ash::parser

#endif // ASH_PARSER_TOKEN_H_
