#ifndef ASH_PARSER_LEXER_H_
#define ASH_PARSER_LEXER_H_

#include "token.h"
#include "ash/source/session.h"

#include <vector>

namespace ash::parser {

class Lexer {
public:
  explicit Lexer(source::Session *session);

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

  source::Session *_session;
  std::string_view _text;
  size_t _index;
  unsigned _line;
  unsigned _column;
  std::vector<Ctx> _context;
  source::LocId _lastLoc;
};

} // namespace ash::parser

#endif // ASH_PARSER_LEXER_H_
