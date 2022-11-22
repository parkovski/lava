#include "lava/lava.h"
#include "lava/syn/syntax.h"

using namespace lava::syn;

#define TYPE_INFO(T, tt) \
  Node::Type T::type() const noexcept { return Node::Type::T; } \
  void T::visit(Visitor &v) { return v.visit_##tt(*this); } \
  void Visitor::visit_##tt(T &node) {}

TYPE_INFO(Leaf, leaf)
TYPE_INFO(Adjacent, adjacent)
TYPE_INFO(Bracketed, bracketed)
TYPE_INFO(Unary, unary)
TYPE_INFO(Infix, infix)

#undef TYPE_INFO

Node::~Node() {}

Leaf::~Leaf() {}

Adjacent::~Adjacent() {}

Bracketed::~Bracketed() {}

Unary::~Unary() {}

Infix::~Infix() {}

// Node spans {{{

RefSpan Leaf::span() const noexcept {
  return _token.span();
}

RefSpan Adjacent::span() const noexcept {
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

// }}}

// Iterator support {{{

unsigned Adjacent::child_count() const noexcept {
  return chain.size();
}

Node *Adjacent::get_child(unsigned n) noexcept {
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

// }}}
