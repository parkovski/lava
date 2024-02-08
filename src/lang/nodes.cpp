#include "lava/lava.h"
#include "lava/lang/nodes.h"
#include "lava/lang/token.h"

using namespace lava::lang;

// ------------------------------------------------------------------------- //

Node::~Node() {}

// ------------------------------------------------------------------------- //

Document::~Document() {}

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

LiteralExpr::~LiteralExpr() {}

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

IdentExpr::~IdentExpr() {}

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

PrefixExpr::~PrefixExpr() {}

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

PostfixExpr::~PostfixExpr() {}

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

BinaryExpr::~BinaryExpr() {}

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

ParenExpr::~ParenExpr() {}

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

InvokeExpr::~InvokeExpr() {}

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

ScopeExpr::~ScopeExpr() {}

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

ReturnExpr::~ReturnExpr() {}

SourceLoc ReturnExpr::start() const {
  return _return.start;
}

SourceLoc ReturnExpr::end() const {
  return _expr ? _expr->end() : _return.end;
}

ExprKind ReturnExpr::expr_kind() const {
  return ExprKind::Return;
}

// ------------------------------------------------------------------------- //

IfExpr::~IfExpr() {}

SourceLoc IfExpr::start() const {
  return _if.start;
}

SourceLoc IfExpr::end() const {
  if (_elses.empty()) {
    return _scope.end();
  } else {
    return _elses.back().end();
  }
}

ExprKind IfExpr::expr_kind() const {
  return ExprKind::If;
}

// ------------------------------------------------------------------------- //

WhileExpr::~WhileExpr() {}

SourceLoc WhileExpr::start() const {
  return _while.start;
}

SourceLoc WhileExpr::end() const {
  return _scope.end();
}

ExprKind WhileExpr::expr_kind() const {
  return ExprKind::While;
}

// ------------------------------------------------------------------------- //

LoopExpr::~LoopExpr() {}


SourceLoc LoopExpr::start() const {
  return _loop.start;
}

SourceLoc LoopExpr::end() const {
  return _scope.end();
}

ExprKind LoopExpr::expr_kind() const {
  return ExprKind::Loop;
}

// ------------------------------------------------------------------------- //

BreakContinueExpr::~BreakContinueExpr() {}

SourceLoc BreakContinueExpr::start() const {
  return _break_or_continue.start;
}

SourceLoc BreakContinueExpr::end() const {
  return _expr ? _expr->end() : _break_or_continue.end;
}

ExprKind BreakContinueExpr::expr_kind() const {
  return ExprKind::BreakContinue;
}

// ------------------------------------------------------------------------- //

NodeKind Item::node_kind() const {
  return NodeKind::Item;
}

// ------------------------------------------------------------------------- //

EmptyItem::~EmptyItem() {}

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

ExprItem::~ExprItem() {}

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

VarDeclItem::~VarDeclItem() {}

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

FunItemBase::~FunItemBase() {}

SourceLoc FunItemBase::start() const {
  return _fun.start;
}

// ------------------------------------------------------------------------- //

FunDeclItem::~FunDeclItem() {}

SourceLoc FunDeclItem::end() const {
  return _semi.end;
}

ItemKind FunDeclItem::item_kind() const {
  return ItemKind::FunDecl;
}

// ------------------------------------------------------------------------- //

FunDefItem::~FunDefItem() {}

SourceLoc FunDefItem::end() const {
  return _body.end();
}

ItemKind FunDefItem::item_kind() const {
  return ItemKind::FunDef;
}

// ------------------------------------------------------------------------- //

StructDefItem::~StructDefItem() {}

SourceLoc StructDefItem::start() const {
  return _struct_or_union.start;
}

SourceLoc StructDefItem::end() const {
  return _rbrace.end;
}

ItemKind StructDefItem::item_kind() const {
  return ItemKind::StructDef;
}
