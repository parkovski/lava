#ifndef LAVA_LANG_LEXER_H_
#define LAVA_LANG_LEXER_H_

#include "token.h"
#include "sourcedoc.h"

namespace lava::lang {

struct Lexer {
private:
  SourceDoc *_doc;
  std::string_view _text;
  SourceLoc _loc;

public:
  explicit Lexer(SourceDoc &doc) noexcept;

  Token lex();

private:
  int getch(unsigned lookahead = 0) const;
  void nextch();

  static int get_keyword(std::string_view word);

  void lex_whitespace(Token &token);
  void lex_line_comment(Token &token);
  void lex_block_comment(Token &token);
  void lex_number(Token &token);
  void lex_hex_number(Token &token);
  void lex_binary_number(Token &token);
  void lex_decimal_part(Token &token);
  void lex_string(Token &token);
  void lex_ident(Token &token);
  void lex_symbol_or_invalid(Token &token);
};

} // namespace lava::lang

#endif // LAVA_LANG_LEXER_H_
