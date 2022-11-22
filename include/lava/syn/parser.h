#ifndef LAVA_SYN_PARSER_H_
#define LAVA_SYN_PARSER_H_

#include "lexer.h"
#include "syntax.h"
#include <boost/container/small_vector.hpp>
#include <optional>

namespace lava::syn {

struct Parser {
  explicit Parser(src::SourceFile &src);

  enum ExprFlags {
    // Last byte is reserved for operator precedence.
    EF_PrecMask   = 0x000000FF,

    // Expression flags - first three bytes.
    EF_FlagMask   = 0xFFFFFF00,

    // Right-to-left parsing, only applies to infix/binary operators.
    EF_RTL        = 0x00000100,

    // Comma not allowed.
    EF_NoComma    = 0x00000200,

    // Don't skip new lines.
    EF_NewLine    = 0x00000400,

    // Parse angle brackets instead of less/greater.
    EF_Angle      = 0x00000800,
  };

  NodePtr operator()(unsigned flags = 0);

private:
  static unsigned prec_prefix(Tk tk, unsigned flags = 0);

  static std::pair<unsigned, unsigned>
  prec_infix_postfix(Tk tk, unsigned flags = 0);

  NodePtr parse_expr(Tree *parent, unsigned flags = 0);
  NodePtr parse_expr_terminal(Tree *parent/*, unsigned flags*/);
  NodePtr parse_expr_prefix(Tree *parent, unsigned flags);
  NodePtr parse_expr_bracketed(Tree *parent, unsigned flags);
  NodePtr parse_expr_left(Tree *parent, NodePtr left, unsigned prec_and_flags);
  NodePtr parse_expr_right(Tree *parent, NodePtr left, unsigned prec_and_flags);

  // NodePtr parse_string();

  //! @returns true to keep parsing trivia, false if EOF was seen and inserted
  //!          into the token queue.
  bool parse_comment_line(bool skip_newline);

  //! @returns true if the comment block was terminated, false if an EOF was
  //!          seen first (block improperly terminated).
  bool parse_comment_block();

  // Take the first token from the queue. If no tokens are available, run the
  // lexer once and return the result directly.
  Token take();

  // Look at a token in the queue. If lookahead is greater than the number of
  // tokens available, run the lexer to get more tokens.
  Token &peek(unsigned lookahead = 0, bool skip_newline = true,
              bool skip_trivia = true);

  Lexer _lexer;

  Token::TriviaList _trivia;

  // Lookahead tokens queue.
  boost::container::small_vector<Token, 2> _tokens;

  // The current token/leaf index. The current number of lookahead tokens is
  // `_leafs.size() - _current_token`.
  unsigned _current_token;
};

} // namespace lava::syn

#endif // LAVA_SYN_PARSER_H_
