#ifndef LAVA_LANG_TOKEN_H_
#define LAVA_LANG_TOKEN_H_

#include <string>
#include <string_view>
#include <cstdint>

namespace lava::lang {

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

#define LAVA_TOKENS \
  X(Eof) \
  \
  X(Whitespace) \
  X(LineComment) \
  X(BlockComment) \
  \
  X(IntLiteral) \
  X(HexLiteral) \
  X(BinLiteral) \
  X(FloatLiteral) \
  X(DoubleLiteral) \
  X(StringLiteral) \
  \
  X(Ident) \
  \
  X(Tilde) \
  X(Excl) \
  X(ExclEq) \
  X(Percent) \
  X(PercentEq) \
  X(Hat) \
  X(HatEq) \
  X(And) \
  X(AndAnd) \
  X(AndAndEq) \
  X(AndEq) \
  X(Star) \
  X(StarStar) \
  X(StarStarEq) \
  X(StarEq) \
  X(LeftParen) \
  X(RightParen) \
  X(Minus) \
  X(MinusMinus) \
  X(MinusEq) \
  X(MinusRightArrow) \
  X(Plus) \
  X(PlusPlus) \
  X(PlusEq) \
  X(Eq) \
  X(EqEq) \
  X(EqRightArrow) \
  X(Or) \
  X(OrOr) \
  X(OrOrEq) \
  X(OrEq) \
  X(LeftSquareBracket) \
  X(RightSquareBracket) \
  X(LeftBrace) \
  X(RightBrace) \
  X(Semi) \
  X(Colon) \
  X(ColonColon) \
  X(Less) \
  X(LessLess) \
  X(LessLessEq) \
  X(LessEq) \
  X(LessMinusLess) \
  X(LessMinusLessEq) \
  X(Greater) \
  X(GreaterGreater) \
  X(GreaterGreaterEq) \
  X(GreaterEq) \
  X(GreaterMinusGreater) \
  X(GreaterMinusGreaterEq) \
  X(Comma) \
  X(Dot) \
  X(DotDot) \
  X(DotDotDot) \
  X(Slash) \
  X(SlashEq) \
  X(Question) \
  \
  X(If) \
  X(Else) \
  X(Switch) \
  X(Case) \
  X(While) \
  X(Loop) \
  X(For) \
  X(In) \
  X(Break) \
  X(Continue) \
  X(Return) \
  X(Fun) \
  X(Struct) \
  X(Union) \
  X(Enum) \
  X(Mut) \
  X(Ref) \

enum {
  TkInvalid = -1,

#define X(Name) Tk##Name,
  LAVA_TOKENS
#undef X
};

std::string_view get_token_name(int what);

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


} // namespace lava::lang

#endif // LAVA_LANG_TOKEN_H_
