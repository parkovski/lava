#ifndef LAVA_PARSER_TOKEN_H_
#define LAVA_PARSER_TOKEN_H_

#include "lava/src/source.h"
#include "location.h"

#include <string_view>
#include <iosfwd>
#include <compare>

#include <fmt/ostream.h>

#include <boost/container/small_vector.hpp>

namespace lava::syn {

#define LAVA_TOKENS(M)   \
  M(Invalid)             \
  M(EndOfInput)          \
  M(Whitespace)          \
  M(NewLine)             \
                         \
  M(HashBang)            \
  M(CommentLine)         \
  M(CommentLineText)     \
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
  M(Excl)                \
  M(ExclEqual)           \
  M(At)                  \
  M(Hash)                \
  M(Percent)             \
  M(PercentEqual)        \
  M(Hat)                 \
  M(HatEqual)            \
  M(And)                 \
  M(AndAnd)              \
  M(AndEqual)            \
  M(Star)                \
  M(StarStar)            \
  M(StarEqual)           \
  M(StarStarEqual)       \
  M(Minus)               \
  M(MinusMinus)          \
  M(MinusEqual)          \
  M(MinusArrowRight)     \
  M(Plus)                \
  M(PlusPlus)            \
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
  K(Enum) \
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
  K(Namespace) \
  K(Or) \
  K(Return) \
  K(Requires) \
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

struct SimpleToken {
  constexpr SimpleToken() noexcept
    : _file{nullptr}
    , _span{}
    , _id{Tk::Invalid}
    , _kw{Kw::_undef}
  {}

  explicit SimpleToken(Tk id, src::SourceFile *file, Span span) noexcept
    : _file{file}
    , _span{span}
    , _id{id}
    , _kw{Kw::_undef}
  {}

  explicit SimpleToken(Kw kw, src::SourceFile *file, Span span) noexcept
    : _file{file}
    , _span{span}
    , _id{Tk::Ident}
    , _kw{kw}
  {}

  SimpleToken(const SimpleToken &) noexcept = default;
  SimpleToken &operator=(const SimpleToken &) noexcept = default;

  bool operator==(const SimpleToken &other) const noexcept = default;

  const Span &span() const noexcept
  { return _span; }

  Tk id() const noexcept
  { return _id; }

  Kw keyword() const noexcept
  { return _kw; }

  std::string_view text() const noexcept
  { return _file->text(_span.start().index, _span.length()); }

  explicit operator bool() const noexcept
  { return bool(_span); }

private:
  src::SourceFile *_file;
  Span _span;
  Tk _id;
  Kw _kw;
};

struct Token : SimpleToken {
  using SimpleToken::SimpleToken;
  using TriviaList = boost::container::small_vector<SimpleToken, 1>;

  explicit Token(const SimpleToken &token, const TriviaList &trivia)
    : SimpleToken{token}
    , _trivia{trivia}
  {}

  Token(const Token &) = default;
  Token &operator=(const Token &) = default;

  Token(Token &&) noexcept = default;
  Token &operator=(Token &&) noexcept = default;

  bool operator==(const Token &other) const noexcept = default;

  TriviaList &trivia() noexcept
  { return _trivia; }

  const TriviaList &trivia() const noexcept
  { return _trivia; }

private:
  TriviaList _trivia;
};

} // namespace lava::source

namespace fmt {
  template<> struct formatter<lava::syn::Tk> : ostream_formatter {};
}

#endif // LAVA_PARSER_TOKEN_H_
