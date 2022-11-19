#include "lava/syn/printer.h"
#include <fmt/format.h>
#include <string_view>

using namespace lava::syn;

namespace {
  std::string_view get_text(lava::src::SourceFile &file, RefSpan span) {
    return std::string_view{&file[span.start().index], span.length()};
  }
}

void Printer::visit_trivia(const TriviaList &trivia) {
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
