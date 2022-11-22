#ifndef LAVA_PARSER_LEXER_H_
#define LAVA_PARSER_LEXER_H_

#include "token.h"
#include "lava/src/source.h"
#include <vector>

namespace lava::syn {

struct Lexer {
  enum class Context : uint8_t {
    Initial,
    EmbedParens,
    EmbedBraces,
    StringBacktick,
    StringDQuote,
    StringSQuote,
    CommentLine,
    CommentBlock,
  };

  struct State {
    Location loc;
    Context ctx;
  };

  explicit Lexer(src::SourceFile &source);

  SimpleToken operator()();
  Location location() const noexcept {
    return _loc;
  }

  src::SourceFile &source() { return *_src; }

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

  char ch(size_t offset = 0) const;
  void fwd(size_t count = 1);
  Context context() const;

  src::SourceFile *_src;
  Location _loc;
  std::vector<Context> _context;
};

} // namespace lava::source

#endif // LAVA_PARSER_LEXER_H_
