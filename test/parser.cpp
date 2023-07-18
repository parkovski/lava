#include <catch2/catch_test_macros.hpp>
#include "lava/syn/parser.h"

using namespace lava::syn;

#define INIT_PARSER(Content) \
  SourceDoc doc{ .name = "test", .content = Content }; \
  Lexer lexer{doc}; \
  Parser parser{lexer}

TEST_CASE("Parser basic", "[syntax][parser][expr]") {
  INIT_PARSER("a+b");

  auto expr = parser.parse_expr();
  REQUIRE(expr->expr_kind() == ExprKind::Binary);
  auto binary = static_cast<BinaryExpr*>(expr.get());
  REQUIRE(binary->op() == TkPlus);
  REQUIRE(binary->left()->expr_kind() == ExprKind::Ident);
  REQUIRE(binary->right()->expr_kind() == ExprKind::Ident);
}

TEST_CASE("Parser simple precedence", "[syntax][parser][expr]") {
  INIT_PARSER("a += b * c + d");

  auto expr = parser.parse_expr();
  REQUIRE(expr->expr_kind() == ExprKind::Binary);
  auto binary = static_cast<const BinaryExpr*>(expr.get());
  REQUIRE(binary->op() == TkPlusEq);
  REQUIRE(binary->left()->expr_kind() == ExprKind::Ident);
  REQUIRE(binary->right()->expr_kind() == ExprKind::Binary);
  binary = static_cast<const BinaryExpr*>(binary->right());
  REQUIRE(binary->op() == TkPlus);
  REQUIRE(binary->left()->expr_kind() == ExprKind::Binary);
  REQUIRE(binary->right()->expr_kind() == ExprKind::Ident);
  binary = static_cast<const BinaryExpr*>(binary->left());
  REQUIRE(binary->op() == TkStar);
  REQUIRE(binary->left()->expr_kind() == ExprKind::Ident);
  REQUIRE(binary->right()->expr_kind() == ExprKind::Ident);
}

TEST_CASE("RTL assignment", "[syntax][parser][expr]") {
  INIT_PARSER("a = b = c + 1");

  auto expr = parser.parse_expr();
  REQUIRE(expr->expr_kind() == ExprKind::Binary);
  auto binary = static_cast<const BinaryExpr*>(expr.get());
  REQUIRE(binary->left()->expr_kind() == ExprKind::Ident);
  REQUIRE(static_cast<const IdentExpr*>(binary->left())->value() == "a");
  REQUIRE(binary->right()->expr_kind() == ExprKind::Binary);
  binary = static_cast<const BinaryExpr*>(binary->right());
  REQUIRE(binary->left()->expr_kind() == ExprKind::Ident);
  REQUIRE(static_cast<const IdentExpr*>(binary->left())->value() == "b");
  REQUIRE(binary->right()->expr_kind() == ExprKind::Binary);
  binary = static_cast<const BinaryExpr*>(binary->right());
  REQUIRE(binary->left()->expr_kind() == ExprKind::Ident);
  REQUIRE(binary->right()->expr_kind() == ExprKind::Literal);
}

TEST_CASE("Parser parens", "[syntax][parser][expr]") {
  INIT_PARSER("(a)*(b+c)");

  auto expr = parser.parse_expr();
  REQUIRE(expr->expr_kind() == ExprKind::Binary);
  auto binary = static_cast<const BinaryExpr*>(expr.get());
  REQUIRE(binary->op() == TkStar);
  REQUIRE(binary->left()->expr_kind() == ExprKind::Paren);
  REQUIRE(binary->right()->expr_kind() == ExprKind::Paren);
  auto paren = static_cast<const ParenExpr*>(binary->left());
  REQUIRE(paren->expr()->expr_kind() == ExprKind::Ident);
  paren = static_cast<const ParenExpr*>(binary->right());
  REQUIRE(paren->expr()->expr_kind() == ExprKind::Binary);
  binary = static_cast<const BinaryExpr*>(paren->expr());
  REQUIRE(binary->op() == TkPlus);
  REQUIRE(binary->left()->expr_kind() == ExprKind::Ident);
  REQUIRE(binary->right()->expr_kind() == ExprKind::Ident);
}

TEST_CASE("Call without args", "[syntax][parser][expr]") {
  INIT_PARSER("foo()");

  auto expr = parser.parse_expr();
  REQUIRE(expr->expr_kind() == ExprKind::Invoke);
  auto invoke = static_cast<const InvokeExpr*>(expr.get());
  REQUIRE(invoke->expr()->expr_kind() == ExprKind::Ident);
  REQUIRE(invoke->args().size() == 0);
};

TEST_CASE("Member call", "[syntax][parser][expr]") {
  INIT_PARSER("foo.bar()");

  auto expr = parser.parse_expr();
  REQUIRE(expr->expr_kind() == ExprKind::Invoke);
  auto invoke = static_cast<InvokeExpr*>(expr.get());
  REQUIRE(invoke->expr()->expr_kind() == ExprKind::Binary);
  REQUIRE(invoke->args().size() == 0);
}

TEST_CASE("Parser call expression", "[syntax][parser][expr]") {
  INIT_PARSER("foo(bar,baz)");

  auto expr = parser.parse_expr();
  REQUIRE(expr->expr_kind() == ExprKind::Invoke);
  auto invoke = static_cast<InvokeExpr*>(expr.get());
  REQUIRE(invoke->expr()->expr_kind() == ExprKind::Ident);
  REQUIRE(invoke->args().size() == 2);
  REQUIRE(invoke->args()[0].value->expr_kind() == ExprKind::Ident);
  REQUIRE(invoke->args()[0].delimiter.has_value());
  REQUIRE(invoke->args()[1].value->expr_kind() == ExprKind::Ident);
  REQUIRE_FALSE(invoke->args()[1].delimiter.has_value());
}

TEST_CASE("Call expression trailing comma", "[syntax][parser][expr]") {
  INIT_PARSER("foo(bar,)");

  auto expr = parser.parse_expr();
  REQUIRE(expr->expr_kind() == ExprKind::Invoke);
  auto invoke = static_cast<InvokeExpr*>(expr.get());
  REQUIRE(invoke->args()[0].value->expr_kind() == ExprKind::Ident);
  REQUIRE(invoke->args()[0].delimiter.has_value());
}

TEST_CASE("Comma inside args", "[syntax][parser][expr]") {
  INIT_PARSER("foo((bar, baz))");

  auto expr = parser.parse_expr();
  REQUIRE(expr->expr_kind() == ExprKind::Invoke);
  auto invoke = static_cast<InvokeExpr*>(expr.get());
  REQUIRE(invoke->expr()->expr_kind() == ExprKind::Ident);
  REQUIRE(invoke->args().size() == 1);
  REQUIRE(invoke->args()[0].value->expr_kind() == ExprKind::Paren);
  auto paren = static_cast<ParenExpr*>(invoke->args()[0].value.get());
  REQUIRE(paren->expr()->expr_kind() == ExprKind::Binary);
  auto binary = static_cast<const BinaryExpr*>(paren->expr());
  REQUIRE(binary->op() == TkComma);
}

TEST_CASE("Var decl", "[syntax][parser][item]") {
  INIT_PARSER("int foo=1, bar=2;");

  auto item = parser.parse_item();
  REQUIRE(item->item_kind() == ItemKind::VarDecl);
  auto var = static_cast<VarDeclItem*>(item.get());
  REQUIRE(var->decls().size() == 2);
  REQUIRE(var->decls()[0].delimiter.has_value());
  REQUIRE_FALSE(var->decls()[1].delimiter.has_value());
}

TEST_CASE("Fun decl", "[syntax][parser][item]") {
  INIT_PARSER("fun foo(int a, int b);");

  auto item = parser.parse_fun_item();
  REQUIRE(item->item_kind() == ItemKind::FunDecl);
  auto fundecl = static_cast<FunDeclItem*>(item.get());
  REQUIRE(fundecl->args().size() == 2);
}

TEST_CASE("Fun def", "[syntax][parser][item]") {
  INIT_PARSER("fun foo() { print('hello'); }");

  auto item = parser.parse_fun_item();
  REQUIRE(item->item_kind() == ItemKind::FunDef);
  auto fundef = static_cast<FunDefItem*>(item.get());
  auto &body = fundef->body();
  REQUIRE(body.exprs()[0].value->expr_kind() == ExprKind::Invoke);
}
