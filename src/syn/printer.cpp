#include "lava/syn/printer.h"
#include <fmt/format.h>
#include <string_view>

using namespace lava::syn;

namespace {
  std::string_view get_text(lava::src::SourceFile &file, RefSpan span) {
    return std::string_view{&file[span.start().index], span.length()};
  }
}

void Printer::visit_trivia(const Token::TriviaList &trivia) {
  for (auto const &tk : trivia) {
    fmt::print("{}", get_text(*_src, tk.span()));
  }
}

void Printer::visit_leaf(Leaf &node) {
  visit_trivia(node.trivia_before());
  fmt::print("{}", get_text(*_src, node.token().span()));
}

void Printer::visit_adjacent(Adjacent &node) {
  for (auto &child : node) {
    child.visit(*this);
  }
}

void Printer::visit_bracketed(Bracketed &node) {
  for (auto &child : node) {
    child.visit(*this);
  }
}

void Printer::visit_unary(Unary &node) {
  for (auto &child : node) {
    child.visit(*this);
  }
}

void Printer::visit_infix(Infix &node) {
  for (auto &child : node) {
    child.visit(*this);
  }
}

void LispPrinter::visit_leaf(Leaf &node) {
  fmt::print("{}", get_text(*_src, node.span()));
}

void LispPrinter::visit_adjacent(Adjacent &node) {
  for (auto &child : node) {
    child.visit(*this);
    fmt::print("\n");
  }
}

void LispPrinter::visit_bracketed(Bracketed &node) {
  //fmt::print("'{}' ", get_text(*_src, node.open->span()));
  node.expr->visit(*this);
  //fmt::print(" '{}'", get_text(*_src, node.close->span()));
}

void LispPrinter::visit_unary(Unary &node) {
  fmt::print("(");
  node.op->visit(*this);
  fmt::print(" ");
  node.expr->visit(*this);
  fmt::print(")");
}

void LispPrinter::visit_infix(Infix &node) {
  fmt::print("(");
  node.chain.front().first->visit(*this);
  fmt::print(" ");
  node.first->visit(*this);
  fmt::print(" ");
  node.chain.front().second->visit(*this);
  for (unsigned i = 1; i < node.chain.size(); ++i) {
    fmt::print(" ");
    node.chain[i].second->visit(*this);
  }
  fmt::print(")");
}
