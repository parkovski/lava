#include <fstream>
#include <optional>
#include <fmt/format.h>
#include "lava/lang/visitor.h"
#include "lava/lang/parser.h"

using namespace lava::lang;

std::optional<std::string> read_file(const char *filename) {
  constexpr size_t buf_size = 4096;
  std::ifstream ifs{filename, std::ios::in | std::ios::binary};

  if (!ifs) {
    return std::nullopt;
  }

  std::string content;
  char buf[buf_size];
  while (ifs.read(buf, buf_size)) {
    content.append(buf, ifs.gcount());
  }
  content.append(buf, ifs.gcount());
  return content;
}

struct Printer : NodeVisitor {
  void visit(const LiteralExpr &expr) override;
  void visit(const IdentExpr &expr) override;
  void visit(const PrefixExpr &expr) override;
  void visit(const PostfixExpr &expr) override;
  void visit(const BinaryExpr &expr) override;
  //void visit(const ParenExpr &expr) override;
  void visit(const InvokeExpr &expr) override;
  void visit(const ScopeExpr &expr) override;

  //void visit(const EmptyItem &item) override;
  //void visit(const ExprItem &item) override;
  void visit(const VarDeclItem &item) override;
  void visit(const FunDeclItem &item) override;
  void visit(const FunDefItem &item) override;
  void visit(const StructDefItem &item) override;

  //void visit(const VarDecl &var) override;
  void visit(const ArgDecl &arg) override;
};

void Printer::visit(const LiteralExpr &expr) {
  switch (expr.type()) {
  case LiteralType::Int:
    fmt::print("  ldint {}\n", expr.token().text());
    break;
  case LiteralType::Float:
    fmt::print("  ldflt {}\n", expr.token().text());
    break;
  case LiteralType::Double:
    fmt::print("  lddbl {}\n", expr.token().text());
    break;
  case LiteralType::String:
    fmt::print("  ldstr {}\n", expr.token().text());
    break;
  }
}

void Printer::visit(const IdentExpr &expr) {
  fmt::print("  ldvar {}\n", expr.value());
}

void Printer::visit(const PrefixExpr &expr) {
  NodeVisitor::visit(*expr.expr());
  fmt::print("  {}.prefix\n", get_token_name(expr.op()));
}

void Printer::visit(const PostfixExpr &expr) {
  NodeVisitor::visit(*expr.expr());
  fmt::print("  {}.postfix\n", get_token_name(expr.op()));
}

void Printer::visit(const BinaryExpr &expr) {
  NodeVisitor::visit(*expr.left());
  NodeVisitor::visit(*expr.right());
  fmt::print("  {}.binary\n", get_token_name(expr.op()));
}

void Printer::visit(const InvokeExpr &expr) {
  for (auto const &arg : expr.args()) {
    NodeVisitor::visit(*arg.value);
  }
  NodeVisitor::visit(*expr.expr());
  fmt::print("call\n");
}

void Printer::visit(const ScopeExpr &scope) {
  fmt::print("{{\n");
  for (auto const &expr : scope.exprs()) {
    NodeVisitor::visit(*expr.value);
  }
  fmt::print("}}\n");
}

void Printer::visit(const VarDeclItem &var) {
  NodeVisitor::visit(*var.type());
  for (auto const &decl : var.decls()) {
    fmt::print("  var {}\n", decl.value.name());
  }
}

void Printer::visit(const FunDeclItem &fun) {
  fmt::print("fun {}(", fun.name());
  bool isfirst = true;
  for (auto const &arg : fun.args()) {
    if (isfirst) {
      isfirst = false;
    } else {
      fmt::print(", ");
    }
    visit(arg.value);
  }
  fmt::print(")");
  if (fun.return_type()) {
    fmt::print(" -> ");
    NodeVisitor::visit(*fun.return_type());
  }
  fmt::print(";\n");
}

void Printer::visit(const FunDefItem &fun) {
  fmt::print("fun {}(", fun.name());
  bool isfirst = true;
  for (auto const &arg : fun.args()) {
    if (isfirst) {
      isfirst = false;
    } else {
      fmt::print(", ");
    }
    visit(arg.value);
  }
  fmt::print(") ");
  if (fun.return_type()) {
    fmt::print("-> ");
    NodeVisitor::visit(*fun.return_type());
    fmt::print(" ");
  }
  visit(fun.body());
}

void Printer::visit(const StructDefItem &struc) {
  fmt::print("{} {} {{\n", struc.is_union() ? "union" : "struct", struc.name());
  for (auto const &var : struc.vars()) {
    visit(var);
  }
  fmt::print("}}\n");
}

void Printer::visit(const ArgDecl &arg) {
  NodeVisitor::visit(*arg.type());
  fmt::print(" {}", arg.name());
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    fmt::print(stderr, "Expected filename.\n");
    return 1;
  }
  auto content = read_file(argv[1]);
  if (!content.has_value()) {
    fmt::print(stderr, "Open file error.\n");
    return 1;
  }
  SourceDoc doc{
    argv[1], std::move(content).value()
  };

  Lexer lexer{doc};
  Parser parser{lexer};

  auto document = parser.parse_document();
  Printer printer;
  printer.NodeVisitor::visit(*document);
}
