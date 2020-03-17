#include "ash/parser/parser.h"
#include "ash/ast/writer.h"

#include <catch2/catch.hpp>

#include <fstream>

static const std::string_view program =
R"""(#!/usr/bin/env ash
echo 'Hello'
let foo = 1
echo (x+1, a * b << c? ** -d - - e & x) &
echo 'Goodbye')""";

TEST_CASE("Parser", "[parser]") {
  using namespace ash::parser;
  using namespace ash::source;

  SourceLocator locator;
  Session session(&locator, locator.addFile("test", program));
  Parser parser(&session);
  auto tree = parser.parseExpressionListV();
  std::ofstream out("parse.test.out");
  ash::ast::Writer writer(session, out);
  tree.write(writer);
  INFO("Parser output written to parse.test.out.");
  SUCCEED();
}
