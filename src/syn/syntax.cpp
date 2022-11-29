#include "lava/lava.h"
#include "lava/syn/syntax.h"

using namespace lava::syn;

// Type info {{{

#define LAVA_NODE_TYPE_IMPL(Class, lcname, Parent)                        \
  constinit const std::string_view Class::Tag = #lcname;                  \
  Class::~Class() {}                                                      \
  void Class::visit(Visitor &v)                                           \
  { this->Parent::visit(v); v.visit_##lcname(*this); }                    \
  std::string_view Class::tag() const noexcept                            \
  { return Class::Tag; }                                                  \
  bool Class::isa(std::string_view tag) const noexcept                    \
  { return this->tag().data() == Class::Tag.data() || Parent::isa(tag); } \

LAVA_NODE_TYPE_IMPL(Leaf, leaf, Node);
LAVA_NODE_TYPE_IMPL(List, list, Tree);
LAVA_NODE_TYPE_IMPL(Bracketed, bracketed, Expr);
LAVA_NODE_TYPE_IMPL(Unary, unary, Expr);
LAVA_NODE_TYPE_IMPL(Infix, infix, Expr);
LAVA_NODE_TYPE_IMPL(ItemDecl, item_decl, Tree);
LAVA_NODE_TYPE_IMPL(NamespaceDef, namespace_def, Tree);
LAVA_NODE_TYPE_IMPL(InterfaceDef, interface_def, Tree);
LAVA_NODE_TYPE_IMPL(StructUnionDef, struct_union_def, Tree);
LAVA_NODE_TYPE_IMPL(EnumDef, enum_def, Tree);
LAVA_NODE_TYPE_IMPL(TypeDef, type_def, Tree);
LAVA_NODE_TYPE_IMPL(FunDecl, fun_decl, Tree);
LAVA_NODE_TYPE_IMPL(FunDef, fun_def, Tree);
LAVA_NODE_TYPE_IMPL(VarDef, var_def, Tree);

// Node {{{

constinit const std::string_view Node::Tag = "node";

Node::~Node() {}

void Node::visit(Visitor &v) {
  v.visit_node(*this);
}

std::string_view Node::tag() const noexcept {
  return Node::Tag;
}

bool Node::isa(std::string_view tag) const noexcept {
  return tag.data() == Node::Tag.data();
}

// }}}

// Tree {{{

constinit const std::string_view Tree::Tag = "tree";

void Tree::visit(Visitor &v) {
  this->Node::visit(v);
  v.visit_tree(*this);
}

std::string_view Tree::tag() const noexcept {
  return Tree::Tag;
}

bool Tree::isa(std::string_view tag) const noexcept {
  return tag.data() == Tree::Tag.data() || Node::isa(tag);
}

// }}}

// Expr {{{

constinit const std::string_view Expr::Tag = "expr";

void Expr::visit(Visitor &v) {
  this->Tree::visit(v);
  v.visit_expr(*this);
}

std::string_view Expr::tag() const noexcept {
  return Expr::Tag;
}

bool Expr::isa(std::string_view tag) const noexcept {
  return tag.data() == Expr::Tag.data() || Tree::isa(tag);
}

// }}}

// }}}

// Default visitor {{{

void Visitor::visit_node(Node &node) {}
void Visitor::visit_leaf(Leaf &node) {}

void Visitor::visit_tree(Tree &node) {
  for (auto &child : node) {
    child.visit(*this);
  }
}

void Visitor::visit_list(List &node) {}
void Visitor::visit_expr(Expr &node) {}
void Visitor::visit_bracketed(Bracketed &node) {}
void Visitor::visit_unary(Unary &node) {}
void Visitor::visit_infix(Infix &node) {}
void Visitor::visit_item_decl(ItemDecl &node) {}
void Visitor::visit_namespace_def(NamespaceDef &node) {}
void Visitor::visit_interface_def(InterfaceDef &node) {}
void Visitor::visit_struct_union_def(StructUnionDef &node) {}
void Visitor::visit_enum_def(EnumDef &node) {}
void Visitor::visit_type_def(TypeDef &node) {}
void Visitor::visit_fun_decl(FunDecl &node) {}
void Visitor::visit_fun_def(FunDef &node) {}
void Visitor::visit_var_def(VarDef &node) {}

// }}}

// Node spans {{{

RefSpan Leaf::span() const noexcept {
  return _token.span();
}

RefSpan List::span() const noexcept {
  return {chain.front()->span().start(), chain.back()->span().end()};
}

RefSpan Bracketed::span() const noexcept {
  return RefSpan{open->span().start(), close->span().end()};
}

RefSpan Unary::span() const noexcept {
  if (is_postfix) {
    return RefSpan{expr->span().start(), op->span().end()};
  } else {
    return RefSpan{op->span().start(), expr->span().end()};
  }
}

RefSpan Infix::span() const noexcept {
  if (is_right_recursive) {
    return RefSpan{chain.front().second->span().start(), first->span().end()};
  } else {
    return RefSpan{first->span().start(), chain.back().second->span().end()};
  }
}

RefSpan ItemDecl::span() const noexcept {
  if (item_list.empty()) {
    return RefSpan{item_word.span().start(), first_item.span().end()};
  } else {
    return RefSpan{
      item_word.span().start(),
      item_list.back().second.span().end()
    };
  }
}

RefSpan NamespaceDef::span() const noexcept {
  return RefSpan{ns_word.span().start(), close_brace.span().end()};
}

RefSpan InterfaceDef::span() const noexcept {
  return RefSpan{interface_word.span().start(), close_brace.span().end()};
}

RefSpan StructUnionDef::span() const noexcept {
  return RefSpan{struct_union_word.span().start(), close_brace.span().end()};
}

RefSpan EnumDef::span() const noexcept {
  return RefSpan{enum_word.span().start(), close_brace.span().end()};
}

RefSpan TypeDef::span() const noexcept {
  return RefSpan{type_word.span().start(), semi.span().end()};
}

RefSpan FunDecl::span() const noexcept {
  return RefSpan{fun_word.span().start(), semi.span().end()};
}

RefSpan FunDef::span() const noexcept {
  return RefSpan{fun_word.span().start(), close_brace.span().end()};
}

RefSpan VarDef::span() const noexcept {
  return RefSpan{var_word.span().start(), semi.span().end()};
}

// }}}

// Iterator support {{{

unsigned List::child_count() const noexcept {
  return chain.size();
}

Node *List::get_child(unsigned n) noexcept {
  return chain[n].get();
}

unsigned Bracketed::child_count() const noexcept {
  return 3;
}

Node *Bracketed::get_child(unsigned n) noexcept {
  switch (n) {
  case 0: return open.get();
  case 1: return expr.get();
  case 2: return close.get();
  default: LAVA_UNREACHABLE();
  }
}

unsigned Unary::child_count() const noexcept {
  return 2;
}

Node *Unary::get_child(unsigned n) noexcept {
  if (is_postfix ^ (n == 0)) return op.get();
  return expr.get();
}

unsigned Infix::child_count() const noexcept {
  return 1 + chain.size() * 2;
}

Node *Infix::get_child(unsigned n) noexcept {
  std::pair<NodePtr, NodePtr> *pair;
  if (is_right_recursive) {
    if (n == chain.size() * 2) return first.get();
    pair = &chain[chain.size() - (n / 2) - 1];
  } else {
    if (n == 0) return first.get();
    pair = &chain[(n - 1) / 2];
  }
  if (n & 1) return pair->first.get();
  else return pair->second.get();
}

unsigned ItemDecl::child_count() const noexcept {
  return 2 + item_list.size() * 2;
}

Node *ItemDecl::get_child(unsigned n) noexcept {
  switch (n) {
  case 0:
    return &item_word;

  case 1:
    return &first_item;

  default:
    if (n & 1) {
      return &item_list[(n - 2) / 2].second;
    } else {
      return &item_list[(n - 2) / 2].first;
    }
  }
}

unsigned NamespaceDef::child_count() const noexcept {
  return 5;
}

Node *NamespaceDef::get_child(unsigned n) noexcept {
  switch (n) {
  case 0: return &ns_word;
  case 1: return &path;
  case 2: return &open_brace;
  case 3: return body.get();
  case 4: return &close_brace;
  default: return nullptr;
  }
}

unsigned InterfaceDef::child_count() const noexcept {
  return 5;
}

Node *InterfaceDef::get_child(unsigned n) noexcept {
  switch (n) {
  case 0: return &interface_word;
  case 1: return &name;
  case 2: return &open_brace;
  case 3: return body.get();
  case 4: return &close_brace;
  default: return nullptr;
  }
}

unsigned StructUnionDef::child_count() const noexcept {
  return 5;
}

Node *StructUnionDef::get_child(unsigned n) noexcept {
  switch (n) {
  case 0: return &struct_union_word;
  case 1: return &name;
  case 2: return &open_brace;
  case 3: return body.get();
  case 4: return &close_brace;
  default: return nullptr;
  }
}

unsigned EnumDef::child_count() const noexcept {
  return 5;
}

Node *EnumDef::get_child(unsigned n) noexcept {
  switch (n) {
  case 0: return &enum_word;
  case 1: return &name;
  case 2: return &open_brace;
  case 3: return body.get();
  case 4: return &close_brace;
  default: return nullptr;
  }
}

unsigned TypeDef::child_count() const noexcept {
  return 5;
}

Node *TypeDef::get_child(unsigned n) noexcept {
  switch (n) {
  case 0: return &type_word;
  case 1: return &name;
  case 2: return &eq;
  case 3: return target.get();
  case 4: return &semi;
  default: return nullptr;
  }
}

unsigned FunDecl::child_count() const noexcept {
  return 6;
}

Node *FunDecl::get_child(unsigned n) noexcept {
  switch (n) {
  case 0: return &fun_word;
  case 1: return &name;
  case 2: return &open_paren;
  case 3: return param_list.get();
  case 4: return &close_paren;
  case 5: return &semi;
  default: return nullptr;
  }
}

unsigned FunDef::child_count() const noexcept {
  return 8;
}

Node *FunDef::get_child(unsigned n) noexcept {
  switch (n) {
  case 0: return &fun_word;
  case 1: return &name;
  case 2: return &open_paren;
  case 3: return param_list.get();
  case 4: return &close_paren;
  case 5: return &open_brace;
  case 6: return body.get();
  case 7: return &close_brace;
  default: return nullptr;
  }
}

unsigned VarDef::child_count() const noexcept {
  return 3;
}

Node *VarDef::get_child(unsigned n) noexcept {
  switch (n) {
  case 0: return &var_word;
  case 1: return &name;
  case 2: return &semi;
  default: return nullptr;
  }
}

// }}}
