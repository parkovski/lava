#include <catch2/catch_test_macros.hpp>
#include "lava/syn/parser.h"

using namespace lava::syn;

TEST_CASE("Parser basic", "[syntax]") {
  SourceDoc doc{
    .name = "test",
    .content = "a+b",
  };
  Lexer lexer{doc};
  Parser parser{lexer};
  auto expr = parser.parse_expr();
  REQUIRE(expr->kind == Expr::Binary);
  auto binary = static_cast<BinaryExpr*>(expr.get());
  REQUIRE(binary->op.what == TkPlus);
  REQUIRE(binary->left->kind == Expr::Ident);
  REQUIRE(binary->right->kind == Expr::Ident);
}

TEST_CASE("Parser simple precedence", "[syntax]") {
  SourceDoc doc{
    .name = "test",
    .content = "a += b * c + d",
  };
  Lexer lexer{doc};
  Parser parser{lexer};
  auto expr = parser.parse_expr();
  REQUIRE(expr->kind == Expr::Binary);
  auto binary = static_cast<BinaryExpr*>(expr.get());
  REQUIRE(binary->op.what == TkPlusEq);
  REQUIRE(binary->left->kind == Expr::Ident);
  REQUIRE(binary->right->kind == Expr::Binary);
  binary = static_cast<BinaryExpr*>(binary->right.get());
  REQUIRE(binary->op.what == TkPlus);
  REQUIRE(binary->left->kind == Expr::Binary);
  REQUIRE(binary->right->kind == Expr::Ident);
  binary = static_cast<BinaryExpr*>(binary->left.get());
  REQUIRE(binary->op.what == TkStar);
  REQUIRE(binary->left->kind == Expr::Ident);
  REQUIRE(binary->right->kind == Expr::Ident);
}

TEST_CASE("Parser parens", "[syntax]") {
  SourceDoc doc{
    .name = "test",
    .content = "(a)*(b+c)",
  };
  Lexer lexer{doc};
  Parser parser{lexer};
  auto expr = parser.parse_expr();
  REQUIRE(expr->kind == Expr::Binary);
  auto binary = static_cast<BinaryExpr*>(expr.get());
  REQUIRE(binary->op.what == TkStar);
  REQUIRE(binary->left->kind == Expr::Paren);
  REQUIRE(binary->right->kind == Expr::Paren);
  auto paren = static_cast<ParenExpr*>(binary->left.get());
  REQUIRE(paren->expr->kind == Expr::Ident);
  paren = static_cast<ParenExpr*>(binary->right.get());
  REQUIRE(paren->expr->kind == Expr::Binary);
  binary = static_cast<BinaryExpr*>(paren->expr.get());
  REQUIRE(binary->op.what == TkPlus);
  REQUIRE(binary->left->kind == Expr::Ident);
  REQUIRE(binary->right->kind == Expr::Ident);
}

TEST_CASE("Call without args", "[syntax]") {
  SourceDoc doc{
    .name = "test",
    .content = "foo()",
  };
  Lexer lexer{doc};
  Parser parser{lexer};
  auto expr = parser.parse_expr();
  REQUIRE(expr->kind == Expr::Invoke);
  auto invoke = static_cast<InvokeExpr*>(expr.get());
  REQUIRE(invoke->left->kind == Expr::Ident);
  REQUIRE(invoke->args.size() == 0);
};

TEST_CASE("Member call", "[syntax]") {
  SourceDoc doc{
    .name = "test",
    .content = "foo.bar()",
  };
  Lexer lexer{doc};
  Parser parser{lexer};
  auto expr = parser.parse_expr();
  REQUIRE(expr->kind == Expr::Invoke);
  auto invoke = static_cast<InvokeExpr*>(expr.get());
  REQUIRE(invoke->left->kind == Expr::Binary);
  REQUIRE(invoke->args.size() == 0);
}

TEST_CASE("Parser call expression", "[syntax]") {
  SourceDoc doc{
    .name = "test",
    .content = "foo(bar,baz)",
  };
  Lexer lexer{doc};
  Parser parser{lexer};
  auto expr = parser.parse_expr();
  REQUIRE(expr->kind == Expr::Invoke);
  auto invoke = static_cast<InvokeExpr*>(expr.get());
  REQUIRE(invoke->left->kind == Expr::Ident);
  REQUIRE(invoke->args.size() == 2);
  REQUIRE(invoke->args[0].expr->kind == Expr::Ident);
  REQUIRE(invoke->args[0].delimiter.has_value());
  REQUIRE(invoke->args[1].expr->kind == Expr::Ident);
  REQUIRE_FALSE(invoke->args[1].delimiter.has_value());
}

TEST_CASE("Call expression trailing comma", "[syntax]") {
  SourceDoc doc{
    .name = "test",
    .content = "foo(bar,)",
  };
  Lexer lexer{doc};
  Parser parser{lexer};
  auto expr = parser.parse_expr();
  REQUIRE(expr->kind == Expr::Invoke);
  auto invoke = static_cast<InvokeExpr*>(expr.get());
  REQUIRE(invoke->args[0].expr->kind == Expr::Ident);
  REQUIRE(invoke->args[0].delimiter.has_value());
}

TEST_CASE("Comma inside args", "[syntax]") {
  SourceDoc doc{
    .name = "test",
    .content = "foo((bar, baz))",
  };
  Lexer lexer{doc};
  Parser parser{lexer};
  auto expr = parser.parse_expr();
  REQUIRE(expr->kind == Expr::Invoke);
  auto invoke = static_cast<InvokeExpr*>(expr.get());
  REQUIRE(invoke->left->kind == Expr::Ident);
  REQUIRE(invoke->args.size() == 1);
  REQUIRE(invoke->args[0].expr->kind == Expr::Paren);
  auto paren = static_cast<ParenExpr*>(invoke->args[0].expr.get());
  REQUIRE(paren->expr->kind == Expr::Binary);
  auto binary = static_cast<BinaryExpr*>(paren->expr.get());
  REQUIRE(binary->op.what == TkComma);
}
