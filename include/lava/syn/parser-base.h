#ifndef LAVA_SYN_PARSER_BASE_H_
#define LAVA_SYN_PARSER_BASE_H_

#include "lexer.h"

#include <vector>
#include <stdexcept>

namespace lava::syn {

struct ParseError : std::logic_error {
  friend struct Parser;

  ParseError(Location loc, const std::string &what)
    : std::logic_error{what}
  {}

  const Location &where() const noexcept
  { return _loc; }

private:
  Location _loc{};
};

struct ParserBase {
  ParserBase(src::SourceFile &src)
    : _lexer{src}
    , _trivia{}
    , _tokens{}
    , _current_token{0}
  {}

protected:
  [[noreturn]] void error(const std::string &msg)
  { throw ParseError(peek().span().start(), msg); }

  void expect(Tk tk, const std::string &msg)
  { if (peek().id() != tk) error(msg); }

  /// @returns true to keep parsing trivia, false if EOF was seen and inserted
  ///          into the token queue.
  bool parse_comment_line(bool skip_newline);

  /// @returns true if the comment block was terminated, false if an EOF was
  ///          seen first (block improperly terminated).
  bool parse_comment_block();

  Token take();

  Token &peek(unsigned lookahead = 0, bool skip_newline = true);

  Lexer _lexer;
  Token::TriviaList _trivia;
  std::vector<Token> _tokens;
  unsigned _current_token;
};

} // namespace lava::syn

#endif // LAVA_SYN_PARSER_BASE_H_
