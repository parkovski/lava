#include "lava/syn/resolver.h"

using namespace lava::syn;

void Resolver::resolve(const Document &doc) {
  for (auto const &item : doc.items()) {
    resolve(*item.get());
  }
}

// ------------------------------------------------------------------------- //

void Resolver::resolve(const Item &item) {
  switch (item.item_kind()) {
  case ItemKind::Empty:
    break;

  case ItemKind::Expr:
    resolve(static_cast<const ExprItem&>(item));
    break;

  case ItemKind::VarDecl:
    resolve(static_cast<const VarDeclItem&>(item));
    break;

  case ItemKind::FunDecl:
    resolve(static_cast<const FunDeclItem&>(item));
    break;

  case ItemKind::FunDef:
    resolve(static_cast<const FunDefItem&>(item));
    break;
  }
}

void Resolver::resolve(const ExprItem &item) {
}

void Resolver::resolve(const VarDeclItem &item) {
}

void Resolver::resolve(const FunDeclItem &item) {
}

void Resolver::resolve(const FunDefItem &item) {
}

// ------------------------------------------------------------------------- //

void Resolver::resolve(const Expr &expr) {
  switch (expr.expr_kind()) {
  case ExprKind::Literal:
    resolve(static_cast<const LiteralExpr&>(expr));
    break;

  case ExprKind::Ident:
    resolve(static_cast<const IdentExpr&>(expr));
    break;

  case ExprKind::Prefix:
    resolve(static_cast<const PrefixExpr&>(expr));
    break;

  case ExprKind::Postfix:
    resolve(static_cast<const PostfixExpr&>(expr));
    break;

  case ExprKind::Binary:
    resolve(static_cast<const BinaryExpr&>(expr));
    break;

  case ExprKind::Paren:
    resolve(static_cast<const ParenExpr&>(expr));
    break;

  case ExprKind::Invoke:
    resolve(static_cast<const InvokeExpr&>(expr));
    break;

  case ExprKind::Scope:
    resolve(static_cast<const ScopeExpr&>(expr));
    break;
  }
}

void Resolver::resolve(const LiteralExpr &expr) {
}

void Resolver::resolve(const IdentExpr &expr) {
}

void Resolver::resolve(const PrefixExpr &expr) {
}

void Resolver::resolve(const PostfixExpr &expr) {
}

void Resolver::resolve(const BinaryExpr &expr) {
}

void Resolver::resolve(const ParenExpr &expr) {
}

void Resolver::resolve(const InvokeExpr &expr) {
}

void Resolver::resolve(const ScopeExpr &expr) {
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
  return scope->get_type(expr.value());
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
  return scope->get_scope(expr.value());
}

Scope *Resolver::resolve_scope(Scope *scope, const BinaryExpr &expr) {
  if (expr.op() == TkDot) {
    scope = resolve_scope(scope, *expr.left());
    return resolve_scope(scope, *expr.right());
  }
  return nullptr;
}
