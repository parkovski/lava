#include <catch2/catch_test_macros.hpp>
#include "lava/syn/symtab.h"

using namespace lava::syn;

TEST_CASE("Namespaces", "[symtab]") {
  SymbolTable symtab;
  auto foo = symtab.global_ns().add_symbol<Namespace>("foo", &symtab.global_ns());
  REQUIRE(foo != nullptr);
  auto bar = symtab.global_ns().add_symbol<Namespace>("bar", &symtab.global_ns());
  REQUIRE(bar != nullptr);
  foo = symtab.global_ns().add_symbol<Namespace>("foo", &symtab.global_ns());
  REQUIRE(foo == nullptr);
  foo = bar->add_symbol<Namespace>("foo", bar);
  REQUIRE(foo != nullptr);

  auto foo2 = symtab.global_ns()["foo"];
  REQUIRE(foo2 != nullptr);
  foo2 = (*static_cast<Namespace*>(symtab.global_ns()["bar"]))["foo"];
  REQUIRE(foo != nullptr);
}

TEST_CASE("Namespaced types", "[symtab]") {
  SymbolTable symtab;
  auto svoid = symtab.global_ns()["void"];
  REQUIRE(svoid != nullptr);
  REQUIRE(svoid->symbol_kind() == SymbolKind::Type);

  auto foo = symtab.global_ns().add_symbol<Namespace>("foo", &symtab.global_ns());
  auto bar = foo->add_symbol<StructType>("bar", std::vector<StructField>{});
  REQUIRE(bar != nullptr);
}
