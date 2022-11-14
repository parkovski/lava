#ifndef LAVA_PARSER_LEXER_H_
#define LAVA_PARSER_LEXER_H_

#include "token.h"
#include <vector>

namespace lava::source {

struct Lexer {
  explicit Lexer(Source &source);

  Token operator()();

private:
  Tk readInitial();
  Tk readEmbedParens();
  Tk readEmbedBraces();
  Tk readStringBacktick();
  Tk readStringDQuote();
  Tk readStringSQuote();
  Tk readCommentLine();
  Tk readCommentBlock();

  Tk readWhitespace();
  Tk readIdent();
  Tk readNumber();
  Tk readEscape();
  Tk readVariable();

  enum class Ctx : uint8_t {
    Initial,
    EmbedParens,
    EmbedBraces,
    StringBacktick,
    StringDQuote,
    StringSQuote,
    CommentLine,
    CommentBlock,
  };

  char ch(size_t offset = 0) const;
  void fwd(size_t count = 1);
  Ctx context() const;

  std::string_view _text;
  size_t _index;
  unsigned _line;
  unsigned _column;
  std::vector<Ctx> _context;
  source::LocId _lastLoc;
};

} // namespace lava::source

#endif // LAVA_PARSER_LEXER_H_
