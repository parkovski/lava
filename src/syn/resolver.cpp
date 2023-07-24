#include "lava/syn/resolver.h"

using namespace lava::syn;

void Resolver::resolve_document(const Document &doc) {
  for (auto const &item : doc.items()) {
    resolve_item(*item.get());
  }
}

// ------------------------------------------------------------------------- //

void Resolver::resolve_item(const Item &item) {
  switch (item.item_kind()) {
  case ItemKind::Empty:
    break;

  case ItemKind::Expr:
    resolve_item(static_cast<const ExprItem&>(item));
    break;

  case ItemKind::VarDecl:
    resolve_item(static_cast<const VarDeclItem&>(item));
    break;

  case ItemKind::FunDecl:
    resolve_item(static_cast<const FunDeclItem&>(item));
    break;

  case ItemKind::FunDef:
    resolve_item(static_cast<const FunDefItem&>(item));
    break;
  }
}

void Resolver::resolve_item(const ExprItem &item) {
}

void Resolver::resolve_item(const VarDeclItem &item) {
}

void Resolver::resolve_item(const FunDeclItem &item) {
}

void Resolver::resolve_item(const FunDefItem &item) {
}

// ------------------------------------------------------------------------- //

void Resolver::resolve_expr(const Expr &expr) {
  switch (expr.expr_kind()) {
  case ExprKind::Literal:
    resolve_expr(static_cast<const LiteralExpr&>(expr));
    break;

  case ExprKind::Ident:
    resolve_expr(static_cast<const IdentExpr&>(expr));
    break;

  case ExprKind::Prefix:
    resolve_expr(static_cast<const PrefixExpr&>(expr));
    break;

  case ExprKind::Postfix:
    resolve_expr(static_cast<const PostfixExpr&>(expr));
    break;

  case ExprKind::Binary:
    resolve_expr(static_cast<const BinaryExpr&>(expr));
    break;

  case ExprKind::Paren:
    resolve_expr(static_cast<const ParenExpr&>(expr));
    break;

  case ExprKind::Invoke:
    resolve_expr(static_cast<const InvokeExpr&>(expr));
    break;

  case ExprKind::Scope:
    resolve_expr(static_cast<const ScopeExpr&>(expr));
    break;
  }
}

void Resolver::resolve_expr(const LiteralExpr &expr) {
}

void Resolver::resolve_expr(const IdentExpr &expr) {
}

void Resolver::resolve_expr(const PrefixExpr &expr) {
}

void Resolver::resolve_expr(const PostfixExpr &expr) {
}

void Resolver::resolve_expr(const BinaryExpr &expr) {
}

void Resolver::resolve_expr(const ParenExpr &expr) {
}

void Resolver::resolve_expr(const InvokeExpr &expr) {
}

void Resolver::resolve_expr(const ScopeExpr &expr) {
}

// ------------------------------------------------------------------------- //

Type *Resolver::resolve_type(Scope *scope, const Expr &expr) {
  switch (expr.expr_kind()) {
  case ExprKind::Ident:
    return resolve_type(scope, static_cast<const IdentExpr&>(expr));

  case ExprKind::Binary:
    return resolve_type(scope, static_cast<const BinaryExpr&>(expr));

  default:
    return nullptr;
  }
}

Type *Resolver::resolve_type(Scope *scope, const IdentExpr &expr) {
  auto symbol = scope->get_symbol(expr.value());
  if (symbol && symbol->symbol_kind() == SymbolKind::Type) {
    return static_cast<Type*>(symbol);
  }
  return nullptr;
}

Type *Resolver::resolve_type(Scope *scope, const BinaryExpr &expr) {
  if (expr.op() == TkDot) {
    scope = resolve_scope(scope, *expr.left());
    if (!scope) {
      return nullptr;
    }
    return resolve_type(scope, *expr.right());
  }
  return nullptr;
}

// ------------------------------------------------------------------------- //

Scope *Resolver::resolve_scope(Scope *scope, const Expr &expr) {
  switch (expr.expr_kind()) {
  case ExprKind::Ident:
    return resolve_scope(scope, static_cast<const IdentExpr&>(expr));

  case ExprKind::Binary:
    return resolve_scope(scope, static_cast<const BinaryExpr&>(expr));

  default:
    return nullptr;
  }
}

Scope *Resolver::resolve_scope(Scope *scope, const IdentExpr &expr) {
  auto symbol = scope->get_symbol(expr.value());
  if (symbol && symbol->symbol_kind() == SymbolKind::Scope) {
    return static_cast<Scope*>(symbol);
  }
  return nullptr;
}

Scope *Resolver::resolve_scope(Scope *scope, const BinaryExpr &expr) {
  if (expr.op() == TkDot) {
    scope = resolve_scope(scope, *expr.left());
    return resolve_scope(scope, *expr.right());
  }
  return nullptr;
}
