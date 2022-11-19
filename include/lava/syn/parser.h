#ifndef LAVA_SYN_PARSER_H_
#define LAVA_SYN_PARSER_H_

#include "lexer.h"
#include "syntax.h"
#include <boost/container/small_vector.hpp>
#include <optional>

namespace lava::syn {

struct Parser {
  explicit Parser(src::SourceFile &src);

  enum ExprFlags : unsigned {
    // Last byte is reserved for operator precedence.
    EF_PrecMask   = 0x000000FF,
    EF_FlagMask   = 0xFFFFFF00,
    // Right-to-left parsing, only applies to infix/binary operators.
    EF_RTL        = 0x80000000,
    // Comma not allowed
    EF_NoComma    = 0x40000000,
  };

  NodePtr operator()();

private:
  bool parse_item();

  bool parse_var_decl();

  bool parse_scoped_id();

  bool parse_fun();
  bool parse_arg_list();
  bool parse_arg();

  bool parse_type();
  bool parse_struct_or_union();

  // Take the first token from the queue. If no tokens are available, run the
  // lexer once and return the result directly.
  Token take();

  // Look at a token in the queue. If lookahead is greater than the number of
  // tokens available, run the lexer to get more tokens.
  const Token &peek(unsigned lookahead = 0);

  Lexer _lexer;

  // Lookahead tokens queue.
  boost::container::small_vector<Token, 2> _tokens;

  // The current token index. The current number of lookahead tokens is
  // `_tokens.size() - _current_token`.
  unsigned _current_token;
};

} // namespace lava::syn

#endif // LAVA_SYN_PARSER_H_
