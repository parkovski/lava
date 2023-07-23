#include <catch2/catch_test_macros.hpp>
#include "lava/syn/resolver.h"
#include "lava/syn/parser.h"

using namespace lava::syn;

#define INIT_RESOLVER(Content) \
  SourceDoc doc{ .name = "test", .content = Content }; \
  Lexer lexer{doc}; \
  Parser parser{lexer}; \
  SymbolTable symtab; \
  Resolver resolver{symtab}; \
  auto expr = parser.parse_expr()

TEST_CASE("Scope resolution", "[resolver]") {
  INIT_RESOLVER("foo.bar.baz");

  symtab.global_scope().add_scope("foo")->add_scope("bar")->add_scope("baz");
  auto scope = resolver.resolve_scope(&symtab.global_scope(), *expr);
  REQUIRE(scope != nullptr);
  REQUIRE(scope->name() == "baz");
}

TEST_CASE("Scope resolution failure", "[resolver]") {
  INIT_RESOLVER("foo.bar.foobar");

  symtab.global_scope().add_scope("foo")->add_scope("bar")->add_scope("baz");
  auto scope = resolver.resolve_scope(&symtab.global_scope(), *expr);
  REQUIRE(scope == nullptr);
}

TEST_CASE("Type resolution", "[resolver]") {
  INIT_RESOLVER("foo.bar.Data");

  Type *type = new DataType{"Data", 1, 1};
  symtab.global_scope().add_scope("foo")->add_scope("bar")
    ->add_named_type(type);
  type = resolver.resolve_type(&symtab.global_scope(), *expr);
  REQUIRE(type != nullptr);
  REQUIRE(type->get_name() == "Data");
}
