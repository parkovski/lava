#include <catch2/catch_test_macros.hpp>
#include "lava/lang/parser.h"
#include "lava/lang/firstpass.h"

using namespace lava::lang;

#define SETUP(Content) \
  PointerType::TargetPointerSize = sizeof(size_t); \
  SymbolTable symtab; \
  FirstPass fp{symtab}; \
  SourceDoc doc{ .name = "test", .content = Content }; \
  Lexer lexer{doc}; \
  Parser parser{lexer}


TEST_CASE("Function declaration", "[firstpass]") {
  SETUP("fun main(int argc);");

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
  SETUP("fun test(int arg); fun test(int arg) {}");

  auto docnode = parser.parse_document();
  REQUIRE(docnode);
  fp.NodeVisitor::visit(*docnode);

  auto test_sym = symtab.global_namespace().get(symtab.intern("test"));
  REQUIRE(test_sym);
  auto test_fn = dynamic_cast<Function*>(test_sym);
  REQUIRE(test_fn);
}

TEST_CASE("Function type equivalence", "[firstpass]") {
  SETUP("fun test(int a); fun test(int b) {}");

  auto docnode = parser.parse_document();
  REQUIRE(docnode);
  fp.NodeVisitor::visit(*docnode);

  auto test_sym = symtab.global_namespace().get(symtab.intern("test"));
  REQUIRE(test_sym);
  auto test_fn = dynamic_cast<Function*>(test_sym);
  REQUIRE(test_fn);

  FunctionType::ArgVector args;
  args.emplace_back(symtab.intern("b"), &symtab.int_type(true));
  auto const &fnty = symtab.function_type(FunctionType{
    &symtab.void_type(),
    std::move(args)
  });
  REQUIRE(*test_fn->type() == fnty);
  REQUIRE(test_fn->type() == &fnty);

  REQUIRE(test_fn->args_namespace().size() == 1);
  REQUIRE(test_fn->args_namespace()[0]->name() == symtab.intern("b"));
}
