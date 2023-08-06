#include <catch2/catch_test_macros.hpp>
#include "lava/lang/lexer.h"

using namespace lava::lang;

#define INIT_LEXER(Content) \
  SourceDoc doc{ "test", Content }; \
  Lexer lexer{doc}

TEST_CASE("Lexer", "[syntax][lexer]") {
  INIT_LEXER("abc +=\n123");

  Token token = lexer.lex();
  REQUIRE(token.what == TkIdent);
  REQUIRE(token.line() == 1);

  token = lexer.lex();
  REQUIRE(token.what == TkWhitespace);

  token = lexer.lex();
  REQUIRE(token.what == TkPlusEq);

  token = lexer.lex();
  REQUIRE(token.what == TkWhitespace);

  token = lexer.lex();
  REQUIRE(token.what == TkIntLiteral);
  REQUIRE(token.line() == 2);

  token = lexer.lex();
  REQUIRE(token.what == TkEof);
}

TEST_CASE("Lex strings", "[syntax][lexer]") {
  INIT_LEXER("'abc' \"123\"");

  Token token = lexer.lex();
  REQUIRE(token.what == TkStringLiteral);
  token = lexer.lex();
  REQUIRE(token.what == TkWhitespace);
  token = lexer.lex();
  REQUIRE(token.what == TkStringLiteral);
  token = lexer.lex();
  REQUIRE(token.what == TkEof);
}
