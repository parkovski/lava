#include <catch2/catch_test_macros.hpp>
#include "lava/syn/lexer.h"

using namespace lava::syn;

TEST_CASE("Lexer", "[syntax][lexer]") {
  SourceDoc doc;
  doc.name = "test";
  doc.content = "abc +=\n123";
  Lexer lexer{doc};

  Token token = lexer.lex();
  REQUIRE(token.what == TkIdent);
  REQUIRE(token.start.line == 1);

  token = lexer.lex();
  REQUIRE(token.what == TkWhitespace);

  token = lexer.lex();
  REQUIRE(token.what == TkPlusEq);

  token = lexer.lex();
  REQUIRE(token.what == TkWhitespace);

  token = lexer.lex();
  REQUIRE(token.what == TkIntLiteral);
  REQUIRE(token.start.line == 2);

  token = lexer.lex();
  REQUIRE(token.what == TkEof);
}
