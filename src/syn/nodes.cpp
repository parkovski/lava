#include "lava/lava.h"
#include "lava/syn/nodes.h"
#include "lava/syn/token.h"

using namespace lava::syn;

NodeKind Document::node_kind() const {
  return NodeKind::Document;
}

SourceLoc Document::start() const {
  if (_items.empty()) {
    return {};
  }
  return _items.front()->start();
}

SourceLoc Document::end() const {
  if (_items.empty()) {
    return {};
  }
  return _items.back()->end();
}

// ------------------------------------------------------------------------- //

NodeKind Expr::node_kind() const {
  return NodeKind::Expr;
}

// ------------------------------------------------------------------------- //

SourceLoc LiteralExpr::start() const {
  return _token.start;
}

SourceLoc LiteralExpr::end() const {
  return _token.end;
}

ExprKind LiteralExpr::expr_kind() const {
  return ExprKind::Literal;
}

// ------------------------------------------------------------------------- //

SourceLoc IdentExpr::start() const {
  return _token.start;
}

SourceLoc IdentExpr::end() const {
  return _token.end;
}

ExprKind IdentExpr::expr_kind() const {
  return ExprKind::Ident;
}

// ------------------------------------------------------------------------- //

SourceLoc PrefixExpr::start() const {
  return _op.start;
}

SourceLoc PrefixExpr::end() const {
  return _expr->end();
}

ExprKind PrefixExpr::expr_kind() const {
  return ExprKind::Prefix;
}

// ------------------------------------------------------------------------- //

SourceLoc PostfixExpr::start() const {
  return _expr->start();
}

SourceLoc PostfixExpr::end() const {
  return _op.end;
}

ExprKind PostfixExpr::expr_kind() const {
  return ExprKind::Postfix;
}

// ------------------------------------------------------------------------- //

SourceLoc BinaryExpr::start() const {
  return _left->start();
}

SourceLoc BinaryExpr::end() const {
  return _right->end();
}

ExprKind BinaryExpr::expr_kind() const {
  return ExprKind::Binary;
}

// ------------------------------------------------------------------------- //

SourceLoc ParenExpr::start() const {
  return _left.start;
}

SourceLoc ParenExpr::end() const {
  return _right.end;
}

ExprKind ParenExpr::expr_kind() const {
  return ExprKind::Paren;
}

// ------------------------------------------------------------------------- //

SourceLoc InvokeExpr::start() const {
  return _expr->start();
}

SourceLoc InvokeExpr::end() const {
  return _rparen.end;
}

ExprKind InvokeExpr::expr_kind() const {
  return ExprKind::Invoke;
}

auto InvokeExpr::bracket_kind() const -> BracketKind {
  switch (_lparen.what) {
  case TkLeftParen:
    return Paren;
  case TkLeftSquareBracket:
    return Square;
  case TkLess:
    return Angle;
  default:
    LAVA_UNREACHABLE();
  }
}

// ------------------------------------------------------------------------- //

SourceLoc ScopeExpr::start() const {
  return _lbrace.start;
}

SourceLoc ScopeExpr::end() const {
  return _rbrace.end;
}

ExprKind ScopeExpr::expr_kind() const {
  return ExprKind::Scope;
}

// ------------------------------------------------------------------------- //

NodeKind Item::node_kind() const {
  return NodeKind::Item;
}

// ------------------------------------------------------------------------- //

SourceLoc EmptyItem::start() const {
  return _semi.start;
}

SourceLoc EmptyItem::end() const {
  return _semi.end;
}

ItemKind EmptyItem::item_kind() const {
  return ItemKind::Empty;
}

// ------------------------------------------------------------------------- //

SourceLoc ExprItem::start() const {
  return _expr->start();
}

SourceLoc ExprItem::end() const {
  return _semi.end;
}

ItemKind ExprItem::item_kind() const {
  return ItemKind::Expr;
}

// ------------------------------------------------------------------------- //

SourceLoc VarDeclItem::start() const {
  return _type->start();
}

SourceLoc VarDeclItem::end() const {
  return _semi.end;
}

ItemKind VarDeclItem::item_kind() const {
  return ItemKind::VarDecl;
}

// ------------------------------------------------------------------------- //

SourceLoc FunItemBase::start() const {
  return _fun.start;
}

// ------------------------------------------------------------------------- //

SourceLoc FunDeclItem::end() const {
  return _semi.end;
}

ItemKind FunDeclItem::item_kind() const {
  return ItemKind::FunDecl;
}

// ------------------------------------------------------------------------- //

SourceLoc FunDefItem::end() const {
  return _body.end();
}

ItemKind FunDefItem::item_kind() const {
  return ItemKind::FunDef;
}
