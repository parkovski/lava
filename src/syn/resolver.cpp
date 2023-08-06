#include "lava/lava.h"
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
  resolve_expr(*item.expr());
}

void Resolver::resolve_item(const VarDeclItem &item) {
  auto type = resolve_type(_scope, *item.type());
  if (!type) {
    return;
  }
  // for (auto const &decl : item.decls()) {
  //   _scope->add_symbol<Variable>(decl.value.name(), type);
  // }
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
#if 0
  auto &instr = _scope->push_instr();
  instr.op = Op::Const;

  switch (expr.type()) {
  case LiteralType::Int:
    instr.args.push_back((uint32_t)expr.int_value());
    break;

  case LiteralType::Float:
    instr.args.push_back(expr.float_value());
    break;

  case LiteralType::Double:
    instr.args.push_back(expr.double_value());
    break;

  case LiteralType::String:
    instr.args.push_back(expr.string_value());
    break;
  }
#endif
}

void Resolver::resolve_expr(const IdentExpr &expr) {
}

void Resolver::resolve_expr(const PrefixExpr &expr) {
  switch (expr.op()) {
  case TkComma:
    break;

  case TkDotDot:
    break;

  case TkTilde:
    break;

  case TkExcl:
    break;

  case TkMinus:
    break;

  case TkPlus:
    break;

  case TkStar:
    break;

  case TkStarStar:
    break;

  case TkAnd:
    break;

  case TkMinusMinus:
    break;

  case TkPlusPlus:
    break;

  case TkDot:
    break;

  default:
    LAVA_UNREACHABLE();
  }
}

void Resolver::resolve_expr(const PostfixExpr &expr) {
  switch (expr.op()) {
  case TkComma:
    break;

  case TkDotDot:
    break;

  case TkMinusMinus:
    break;

  case TkPlusPlus:
    break;

  case TkExcl:
    break;

  case TkQuestion:
    break;

  default:
    LAVA_UNREACHABLE();
  }
}

void Resolver::resolve_expr(const BinaryExpr &expr) {
  switch (expr.op()) {
  case TkPercentEq:
    break;

  case TkHatEq:
    break;

  case TkAndEq:
    break;

  case TkStarEq:
    break;

  case TkStarStarEq:
    break;

  case TkMinusEq:
    break;

  case TkPlusEq:
    break;

  case TkEq:
    break;

  case TkOrEq:
    break;

  case TkLessLessEq:
    break;

  case TkLessMinusLessEq:
    break;

  case TkGreaterGreaterEq:
    break;

  case TkGreaterMinusGreaterEq:
    break;

  case TkSlashEq:
    break;

  case TkComma:
    break;

  case TkDotDot:
    break;

  case TkOrOr:
    break;

  case TkAndAnd:
    break;

  case TkEqEq:
    break;

  case TkExclEq:
    break;

  case TkAnd:
    break;

  case TkHat:
    break;

  case TkOr:
    break;

  case TkLess:
    break;

  case TkLessEq:
    break;

  case TkGreater:
    break;

  case TkGreaterEq:
    break;

  case TkLessLess:
    break;

  case TkLessMinusLess:
    break;

  case TkGreaterGreater:
    break;

  case TkGreaterMinusGreater:
    break;

  case TkMinus:
    break;

  case TkPlus:
    break;

  case TkPercent:
    break;

  case TkStar:
    break;

  case TkSlash:
    break;

  case TkStarStar:
    break;

  case TkDot:
    break;

  default:
    LAVA_UNREACHABLE();
  }
}

void Resolver::resolve_expr(const ParenExpr &expr) {
  resolve_expr(*expr.expr());
}

void Resolver::resolve_expr(const InvokeExpr &expr) {
}

void Resolver::resolve_expr(const ScopeExpr &expr) {
#if 0
  auto prev_scope = _scope;
  _scope = _scope->add_scope();
  for (auto const &e : expr.exprs()) {
    resolve_expr(*e.value);
  }
  _scope = prev_scope;
#endif
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
#if 0
  auto symbol = scope->get_symbol(expr.value());
  if (symbol && symbol->symbol_kind() == SymbolKind::Type) {
    return static_cast<Type*>(symbol);
  }
#endif
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
#if 0
  auto symbol = scope->get_symbol(expr.value());
  if (symbol && symbol->symbol_kind() == SymbolKind::Scope) {
    return static_cast<Scope*>(symbol);
  }
#endif
  return nullptr;
}

Scope *Resolver::resolve_scope(Scope *scope, const BinaryExpr &expr) {
  if (expr.op() == TkDot) {
    scope = resolve_scope(scope, *expr.left());
    return resolve_scope(scope, *expr.right());
  }
  return nullptr;
}
