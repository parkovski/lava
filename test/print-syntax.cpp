#include <fstream>
#include <optional>
#include <fmt/format.h>
#include "lava/lang/nodes.h"
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

struct Printer {
  std::string _indent;
  void indent() {
    _indent += "  ";
  }
  void dedent() {
    if (_indent.size() > 2) {
      _indent.resize(_indent.size() - 2);
    } else {
      _indent.resize(0);
    }
  }

  void print_document(const Document &doc) {
    fmt::print("{}document {{\n", _indent);
    indent();
    for (auto const &item: doc.items()) {
      print_item(*item);
    }
    dedent();
    fmt::print("}}\n");
  }

  void print_literal_expr(const LiteralExpr &literal) {
    switch (literal.type()) {
    case LiteralType::Int:
      fmt::print("int {}", literal.token().text());
      break;
    case LiteralType::Float:
      fmt::print("float {}", literal.token().text());
      break;
    case LiteralType::Double:
      fmt::print("double {}", literal.token().text());
      break;
    case LiteralType::String:
      fmt::print("string {}", literal.token().text());
    }
  }

  void print_ident_expr(const IdentExpr &ident) {
    fmt::print("{}", ident.value());
  }

  void print_prefix_expr(const PrefixExpr &prefix) {
    fmt::print("{} ", get_token_name(prefix.op()));
    print_expr(*prefix.expr());
  }

  void print_postfix_expr(const PostfixExpr &postfix) {
    print_expr(*postfix.expr());
    fmt::print(" {}", get_token_name(postfix.op()));
  }

  void print_binary_expr(const BinaryExpr &binary) {
    print_expr(*binary.left());
    fmt::print(" {} ", get_token_name(binary.op()));
    print_expr(*binary.right());
  }

  void print_paren_expr(const ParenExpr &paren) {
    fmt::print("(");
    print_expr(*paren.expr());
    fmt::print(")");
  }

  void print_invoke_expr(const InvokeExpr &invoke) {
    print_expr(*invoke.expr());

    switch (invoke.bracket_kind()) {
    case InvokeExpr::BracketKind::Paren:
      fmt::print("(");
      break;
    case InvokeExpr::BracketKind::Square:
      fmt::print("[");
      break;
    case InvokeExpr::BracketKind::Angle:
      fmt::print("<");
      break;
    }

    bool isfirst = true;
    for (auto const &arg: invoke.args()) {
      if (isfirst) {
        isfirst = false;
      } else {
        fmt::print(", ");
      }
      print_expr(*arg.value);
    }

    switch (invoke.bracket_kind()) {
    case InvokeExpr::BracketKind::Paren:
      fmt::print(")");
      break;
    case InvokeExpr::BracketKind::Square:
      fmt::print("]");
      break;
    case InvokeExpr::BracketKind::Angle:
      fmt::print(">");
      break;
    }
  }

  void print_scope_expr(const ScopeExpr &scope) {
    fmt::print("{{\n", _indent);
    indent();
    for (auto const &expr : scope.exprs()) {
      fmt::print("{}", _indent);
      print_expr(*expr.value);
      fmt::print("\n");
    }
    dedent();
    fmt::print("{}}}\n", _indent);
  }

  void print_return_expr(const ReturnExpr &ret) {
    if (ret.expr()) {
      fmt::print("return ");
      print_expr(*ret.expr());
    } else {
      fmt::print("return");
    }
  }

  void print_if_expr(const IfExpr &if_) {
    fmt::print("if ");
    print_expr(*if_.expr());
    print_scope_expr(if_.scope());

    auto const &elses = if_.elses();
    for (auto const &else_ : elses) {
      if (else_.expr()) {
        fmt::print("{}else if ", _indent);
        print_expr(*else_.expr());
        print_scope_expr(else_.scope());
      } else {
        fmt::print("{}else ", _indent);
        print_scope_expr(else_.scope());
      }
    }
  }

  void print_while_expr(const WhileExpr &while_) {
    fmt::print("while ");
    print_expr(*while_.expr());
    print_scope_expr(while_.scope());
  }

  void print_loop_expr(const LoopExpr &loop) {
    fmt::print("loop ");
    print_scope_expr(loop.scope());
  }

  void print_break_continue_expr(const BreakContinueExpr &bc) {
    if (bc.is_break()) {
      fmt::print("break");
    } else {
      fmt::print("continue");
    }

    if (bc.expr()) {
      fmt::print(" ");
      print_expr(*bc.expr());
    }
  }

  void print_expr(const Expr &expr) {
    switch (expr.expr_kind()) {
    case ExprKind::Literal:
      print_literal_expr(static_cast<const LiteralExpr&>(expr));
      break;
    case ExprKind::Ident:
      print_ident_expr(static_cast<const IdentExpr&>(expr));
      break;
    case ExprKind::Prefix:
      print_prefix_expr(static_cast<const PrefixExpr&>(expr));
      break;
    case ExprKind::Postfix:
      print_postfix_expr(static_cast<const PostfixExpr&>(expr));
      break;
    case ExprKind::Binary:
      print_binary_expr(static_cast<const BinaryExpr&>(expr));
      break;
    case ExprKind::Paren:
      print_paren_expr(static_cast<const ParenExpr&>(expr));
      break;
    case ExprKind::Invoke:
      print_invoke_expr(static_cast<const InvokeExpr&>(expr));
      break;
    case ExprKind::Scope:
      print_scope_expr(static_cast<const ScopeExpr&>(expr));
      break;
    case ExprKind::Return:
      print_return_expr(static_cast<const ReturnExpr&>(expr));
      break;
    case ExprKind::If:
      print_if_expr(static_cast<const IfExpr&>(expr));
      break;
    case ExprKind::While:
      print_while_expr(static_cast<const WhileExpr&>(expr));
      break;
    case ExprKind::Loop:
      print_loop_expr(static_cast<const LoopExpr&>(expr));
      break;
    case ExprKind::BreakContinue:
      print_break_continue_expr(static_cast<const BreakContinueExpr&>(expr));
      break;
    }
  }

  void print_var_decl(const VarDeclItem &var) {
    print_expr(*var.type());
    fmt::print(" ");
    bool isfirst = true;
    for (auto const &decl : var.decls()) {
      if (isfirst) {
        isfirst = false;
      } else {
        fmt::print(", ");
      }
      fmt::print("{}", decl.value.name());
      if (decl.value.init()) {
        fmt::print(" = ");
        print_expr(*decl.value.init().value().expr());
      }
    }
  }

  void print_arg(const ArgDecl &arg) {
    print_expr(*arg.type());
    fmt::print(" {}", arg.name());
    if (arg.init()) {
      fmt::print(" = ");
      print_expr(*arg.init().value().expr());
    }
  }

  void print_fun_item(const FunItemBase &fun) {
    fmt::print("fun {}(", fun.name());
    bool isfirst = true;
    for (auto const &arg: fun.args()) {
      if (isfirst) {
        isfirst = false;
      } else {
        fmt::print(", ");
      }
      print_arg(arg.value);
    }
    fmt::print(")");

    if (fun.return_type()) {
      fmt::print(" -> ");
      print_expr(*fun.return_type());
    }
  }

  void print_fun_decl(const FunDeclItem &fun) {
    print_fun_item(fun);
    fmt::print(";\n");
  }

  void print_fun_def(const FunDefItem &fun) {
    print_fun_item(fun);
    fmt::print(" {{\n");
    indent();
    for (auto const &expr : fun.body().exprs()) {
      fmt::print("{}", _indent);
      print_expr(*expr.value);
      fmt::print("\n");
    }
    dedent();
    fmt::print("{}}}\n", _indent);
  }

  void print_struct_def(const StructDefItem &struc) {
    fmt::print("{} {} {{\n",
               struc.is_union() ? "union" : "struct",
               struc.name());
    indent();
    for (auto const &var : struc.vars()) {
      fmt::print("{}", _indent);
      print_var_decl(var);
      fmt::print("\n");
    }
    dedent();
    fmt::print("{}}}\n", _indent);
  }

  void print_item(const Item &item) {
    switch (item.item_kind()) {
    case ItemKind::Empty:
      fmt::print("{};\n", _indent);
      break;
    case ItemKind::Expr:
      fmt::print("{}", _indent);
      print_expr(*static_cast<const ExprItem&>(item).expr());
      fmt::print("\n");
      break;
    case ItemKind::VarDecl:
      fmt::print("{}", _indent);
      print_var_decl(static_cast<const VarDeclItem&>(item));
      break;
    case ItemKind::FunDecl:
      fmt::print("{}", _indent);
      print_fun_decl(static_cast<const FunDeclItem&>(item));
      break;
    case ItemKind::FunDef:
      fmt::print("{}", _indent);
      print_fun_def(static_cast<const FunDefItem&>(item));
      break;
    case ItemKind::StructDef:
      fmt::print("{}", _indent);
      print_struct_def(static_cast<const StructDefItem&>(item));
      break;
    }
  }
};

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
  printer.print_document(*document);
}
