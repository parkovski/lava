#include "lava/lang/visitor.h"
#include "lava/lang/nodes.h"

using namespace lava::lang;

void NodeVisitor::visit(const Document &doc) {
  for (auto const &item : doc.items()) {
    visit(*item);
  }
}

void NodeVisitor::visit(const Expr &expr) {
  switch (expr.expr_kind()) {
  case ExprKind::Literal:
    visit(static_cast<const LiteralExpr&>(expr));
    break;
  case ExprKind::Ident:
    visit(static_cast<const IdentExpr&>(expr));
    break;
  case ExprKind::Prefix:
    visit(static_cast<const PrefixExpr&>(expr));
    break;
  case ExprKind::Postfix:
    visit(static_cast<const PostfixExpr&>(expr));
    break;
  case ExprKind::Binary:
    visit(static_cast<const BinaryExpr&>(expr));
    break;
  case ExprKind::Paren:
    visit(static_cast<const ParenExpr&>(expr));
    break;
  case ExprKind::Invoke:
    visit(static_cast<const InvokeExpr&>(expr));
    break;
  case ExprKind::Scope:
    visit(static_cast<const ScopeExpr&>(expr));
    break;
  }
}

void NodeVisitor::visit(const LiteralExpr &expr) {}

void NodeVisitor::visit(const IdentExpr &expr) {}

void NodeVisitor::visit(const PrefixExpr &expr) {
  visit(*expr.expr());
}

void NodeVisitor::visit(const PostfixExpr &expr) {
  visit(*expr.expr());
}

void NodeVisitor::visit(const BinaryExpr &expr) {
  visit(*expr.left());
  visit(*expr.right());
}

void NodeVisitor::visit(const ParenExpr &expr) {
  visit(*expr.expr());
}

void NodeVisitor::visit(const InvokeExpr &expr) {
  visit(*expr.expr());
  for (auto const &arg : expr.args()) {
    visit(*arg.value);
  }
}

void NodeVisitor::visit(const ScopeExpr &expr) {
  for (auto const &inner_expr: expr.exprs()) {
    visit(*inner_expr.value);
  }
}

void NodeVisitor::visit(const Item &item) {
  switch (item.item_kind()) {
  case ItemKind::Empty:
    visit(static_cast<const EmptyItem&>(item));
    break;
  case ItemKind::Expr:
    visit(static_cast<const ExprItem&>(item));
    break;
  case ItemKind::VarDecl:
    visit(static_cast<const VarDeclItem&>(item));
    break;
  case ItemKind::FunDecl:
    visit(static_cast<const FunDeclItem&>(item));
    break;
  case ItemKind::FunDef:
    visit(static_cast<const FunDefItem&>(item));
    break;
  case ItemKind::StructDef:
    visit(static_cast<const StructDefItem&>(item));
    break;
  }
}

void NodeVisitor::visit(const EmptyItem &item) {}

void NodeVisitor::visit(const ExprItem &item) {
  visit(*item.expr());
}

void NodeVisitor::visit(const VarDeclItem &item) {
  visit(*item.type());
  for (auto const &decl : item.decls()) {
    visit(decl.value);
  }
}

void NodeVisitor::visit(const FunDeclItem &item) {
  for (auto const &arg : item.args()) {
    visit(arg.value);
  }
  if (item.return_type()) {
    visit(*item.return_type());
  }
}

void NodeVisitor::visit(const FunDefItem &item) {
  for (auto const &arg : item.args()) {
    visit(arg.value);
  }
  if (item.return_type()) {
    visit(*item.return_type());
  }
  visit(item.body());
}

void NodeVisitor::visit(const StructDefItem &item) {
  for (auto const &var : item.vars()) {
    visit(*var.type());
    for (auto const &decl : var.decls()) {
      visit(decl.value);
    }
  }
}

void NodeVisitor::visit(const VarDecl &var) {}

void NodeVisitor::visit(const ArgDecl &arg) {}
