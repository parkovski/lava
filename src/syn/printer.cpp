#include "lava/syn/printer.h"
#include <fmt/format.h>
#include <string_view>

using namespace lava::syn;

namespace {
  std::string_view get_text(lava::src::SourceFile &file, RefSpan span) {
    return std::string_view{&file[span.start().index], span.length()};
  }

  std::string_view get_text(lava::src::SourceFile &file, Node &node) {
    return get_text(file, node.span());
  }
}

// Printer {{{

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

// }}}

// PrinterLisp {{{

void PrinterLisp::visit_leaf(Leaf &node) {
  fmt::print("{}", get_text(*_src, node.span()));
}

void PrinterLisp::visit_adjacent(Adjacent &node) {
  for (auto &child : node) {
    child.visit(*this);
    fmt::print("\n");
  }
}

void PrinterLisp::visit_bracketed(Bracketed &node) {
  //fmt::print("'{}' ", get_text(*_src, node.open->span()));
  node.expr->visit(*this);
  //fmt::print(" '{}'", get_text(*_src, node.close->span()));
}

void PrinterLisp::visit_unary(Unary &node) {
  fmt::print("(");
  node.op->visit(*this);
  fmt::print(" ");
  node.expr->visit(*this);
  fmt::print(")");
}

void PrinterLisp::visit_infix(Infix &node) {
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

// }}}

// PrinterXml {{{

namespace {
  std::string_view get_text_escaped(lava::src::SourceFile &file, Node &node) {
    // TODO Use a real escape function.
    if (node.type() == Node::Type::Leaf) {
      switch (static_cast<Leaf&>(node).token().id()) {
      default: break;
      case Tk::And: return "&amp;"; break;
      case Tk::AndAnd: return "&amp;&amp;"; break;
      case Tk::AndEqual: return "&amp;="; break;
      case Tk::Less: return "&lt;"; break;
      case Tk::LessLess: return "&lt;&lt;"; break;
      case Tk::LessEqual: return "&lt;="; break;
      case Tk::LessLessEqual: return "&lt;&lt;="; break;
      case Tk::Greater: return "&gt;"; break;
      case Tk::GreaterGreater: return "&gt;&gt;"; break;
      case Tk::GreaterEqual: return "&gt;="; break;
      case Tk::GreaterGreaterEqual: return "&gt;&gt;="; break;
      }
    }
    return get_text(file, node);
  }
}

PrinterXml::PrinterXml(lava::src::SourceFile &src)
  : _src{&src}
{
  fmt::print("<?xml version=\"1.0\" encoding=\"UTF-8\" ?>\n");
}

void PrinterXml::visit_leaf(Leaf &node) {
  fmt::print("<leaf kind=\"{}\">{}</leaf>",
             node.token().id(), get_text_escaped(*_src, node));
}

void PrinterXml::visit_adjacent(Adjacent &node) {
  fmt::print("<adjacent>");
  for (auto &child : node) {
    child.visit(*this);
  }
  fmt::print("</adjacent>");
}

void PrinterXml::visit_bracketed(Bracketed &node) {
  auto open = get_text_escaped(*_src, *node.open);
  auto close = get_text_escaped(*_src, *node.close);
  fmt::print("<bracketed kind=\"{}{}\">", open, close);
  node.expr->visit(*this);
  fmt::print("</bracketed>");
}

void PrinterXml::visit_unary(Unary &node) {
  fmt::print("<unary op=\"{}\">", get_text_escaped(*_src, *node.op));
  node.expr->visit(*this);
  fmt::print("</unary>");
}

void PrinterXml::visit_infix(Infix &node) {
  fmt::print("<infix op=\"{}\">",
             get_text_escaped(*_src, *node.chain.begin()->first));
  node.first->visit(*this);
  for (unsigned i = 0; i < node.chain.size(); ++i) {
    node.chain[i].second->visit(*this);
  }
  fmt::print("</infix>");
}

// }}}
