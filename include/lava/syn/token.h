#ifndef LAVA_PARSER_TOKEN_H_
#define LAVA_PARSER_TOKEN_H_

#include "location.h"
#include <string_view>
#include <iosfwd>
#include <fmt/ostream.h>
#include <compare>

namespace lava::syn {

#define LAVA_TOKENS(M)   \
  M(Invalid)             \
  M(EndOfInput)          \
  M(Whitespace)          \
  M(NewLine)             \
                         \
  M(HashBang)            \
  M(CommentLine)         \
  M(CommentBlockStart)   \
  M(CommentBlockText)    \
  M(CommentBlockEnd)     \
  M(CommentKeyword)      \
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
  M(Tilde)               \
  M(Not)                 \
  M(NotEqual)            \
  M(At)                  \
  M(Hash)                \
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
#define LAVA_TK(t) t,
  LAVA_TOKENS(LAVA_TK)
#undef LAVA_TK
};

#define LAVA_TK(t) +1
constexpr uint16_t Tk_count = 0 LAVA_TOKENS(LAVA_TK);
#undef LAVA_TK

std::string_view to_string(Tk tk);
std::ostream &operator<<(std::ostream &os, Tk tk);

#define LAVA_KEYWORDS(K) \
  K(And) \
  K(As) \
  K(Break) \
  K(Const) \
  K(Continue) \
  K(Do) \
  K(Else) \
  K(For) \
  K(Fun) \
  K(If) \
  K(Implement) \
  K(In) \
  K(Interface) \
  K(Is) \
  K(Let) \
  K(Loop) \
  K(Mutable) \
  K(Or) \
  K(Return) \
  K(Struct) \
  K(Then) \
  K(Type) \
  K(Union) \
  K(While) \

enum class Kw : uint16_t {
#define LAVA_KW(k) k,
  LAVA_KEYWORDS(LAVA_KW)
#undef LAVA_KW
  _undef = 0xffff
};

#define LAVA_KW(K) +1
constexpr uint16_t Kw_count = 0 LAVA_KEYWORDS(LAVA_KW);
#undef LAVA_KW

std::string_view to_string(Kw kw);
std::ostream &operator<<(std::ostream &os, Kw kw);
Kw kw_from_string(std::string_view str);

struct Token {
  constexpr Token() noexcept
    : _span{}, _id{Tk::Invalid}, _kw{Kw::_undef}
  {}

  constexpr explicit Token(Tk id, Span span) noexcept
    : _span{span}, _id{id}, _kw{Kw::_undef}
  {}

  constexpr explicit Token(Kw kw, Span span) noexcept
    : _span{span}, _id{Tk::Ident}, _kw{kw}
  {}

  constexpr Token(const Token &) noexcept = default;
  constexpr Token &operator=(const Token &) noexcept = default;

  constexpr bool operator==(const Token &other) const noexcept = default;

  const Span &span() const noexcept
  { return _span; }

  Tk id() const noexcept
  { return _id; }

  Kw keyword() const noexcept
  { return _kw; }

  explicit operator bool() const noexcept
  { return bool(_span); }

private:
  Span _span;
  Tk _id;
  Kw _kw;
};

} // namespace lava::source

namespace fmt {
  template<> struct formatter<lava::syn::Tk> : ostream_formatter {};
}

#endif // LAVA_PARSER_TOKEN_H_
