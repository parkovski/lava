#include <catch2/catch_test_macros.hpp>
#include "lava/lang/parser.h"
#include "lava/lang/firstpass.h"

using namespace lava::lang;

TEST_CASE("Function declaration", "[firstpass]") {
  PointerType::TargetPointerSize = sizeof(size_t);
  SymbolTable symtab;
  FirstPass fp{symtab};
  SourceDoc doc{ .name = "test", .content = "fun main(int argc);" };
  Lexer lexer{doc};
  Parser parser{lexer};

  auto docnode = parser.parse_document();
  REQUIRE(docnode);
  fp.NodeVisitor::visit(*docnode);

  auto main_sym = symtab.global_namespace().get(symtab.intern("main"));
  REQUIRE(main_sym);
  auto main_fn = dynamic_cast<Function*>(main_sym);
  REQUIRE(main_fn);
  REQUIRE(main_fn->args_namespace().get(symtab.intern("argc")));
}

TEST_CASE("Function declaration/definition", "[firstpass]") {
  PointerType::TargetPointerSize = sizeof(size_t);
  SymbolTable symtab;
  FirstPass fp{symtab};
  SourceDoc doc{
    .name = "test",
    .content = "fun test(int arg); fun test(int arg) {}"
  };
  Lexer lexer{doc};
  Parser parser{lexer};

  auto docnode = parser.parse_document();
  REQUIRE(docnode);
  fp.NodeVisitor::visit(*docnode);

  auto test_sym = symtab.global_namespace().get(symtab.intern("test"));
  REQUIRE(test_sym);
  auto test_fn = dynamic_cast<Function*>(test_sym);
  REQUIRE(test_fn);
}
