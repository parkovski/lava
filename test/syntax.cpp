#include <catch2/catch_test_macros.hpp>
#include "lava/lang/syntax.h"

using namespace lava::lang;

#if 0
struct Foo {
  struct Bar {
    int x;
    int y;
  }
  int a;
  int b;
}
#endif
TEST_CASE("String interning", "[syntax]") {
  Program program;
  istring hello = program.intern("hello");
  REQUIRE(hello == "hello");
  istring world = program.intern("world");
  REQUIRE(world == "world");
  istring hello2 = program.intern("hello");
  REQUIRE(hello.data() == hello2.data());
}

TEST_CASE("Symbols", "[symbol]") {
  Program program;
  istring main_str = program.intern("main");
  auto main_mod = program.add_module(main_str);
  REQUIRE(main_mod != nullptr);
  auto main_mod_dup = program.add_module(main_str);
  REQUIRE(main_mod_dup == nullptr);
}
