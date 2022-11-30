#ifndef LAVA_SYN_SYNTAX_H_
#define LAVA_SYN_SYNTAX_H_

#include "location.h"
#include "token.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <string_view>
#include <iterator>
#include <compare>
#include <boost/container/small_vector.hpp>

namespace lava::syn {

struct Visitor;
struct Trivia;
struct Tree;

typedef std::unique_ptr<struct Node> NodePtr;

#define LAVA_EXTENDS_TREE(TypeName)                             \
  static constinit const std::string_view Tag;                  \
  TypeName() = default;                                         \
  TypeName(const TypeName &) = delete;                          \
  TypeName &operator=(const TypeName &) = delete;               \
  TypeName(TypeName &&) = default;                              \
  TypeName &operator=(TypeName &&) = default;                   \
  ~TypeName();                                                  \
  RefSpan span() const noexcept override final;                 \
  void visit(Visitor &v) override final;                        \
  std::string_view tag() const noexcept override final;         \
  bool isa(std::string_view tag) const noexcept override final; \
  unsigned child_count() const noexcept override final;         \
  Node *get_child(unsigned n) noexcept override final           \

// Base classes {{{

// Syntax tree base type.
struct Node {
  static constinit const std::string_view Tag;

  virtual ~Node() = 0;

  // The root node is the only node with no parent.
  bool is_root() const noexcept
  { return _parent == nullptr; }

  // Set the parent node after construction.
  void set_parent(Tree *parent) noexcept
  { _parent = parent; }

  // Get this node's parent node. Always defined unless `is_root()` is true.
  Tree *parent() noexcept
  { return _parent; }

  // Get this node's parent node. Always defined unless `is_root()` is true.
  const Tree *parent() const noexcept
  { return _parent; }

  // Total source span of all the tokens in this node.
  virtual RefSpan span() const noexcept = 0;

  // Visit this node.
  virtual void visit(Visitor &v);

  // Runtime type info - type name.
  virtual std::string_view tag() const noexcept = 0;

  // Runtime type info - valid inputs are constinit static Tag members only.
  virtual bool isa(std::string_view tag) const noexcept;

  // Runtime type info test.
  template<class T>
  bool isa() const noexcept
  { return isa(T::Tag); }

private:
  // Parent node; only the root node may be null.
  Tree *_parent = nullptr;
};

struct Tree : Node {
  static constinit const std::string_view Tag;

  void visit(Visitor &v) override;

  std::string_view tag() const noexcept override;

  bool isa(std::string_view tag) const noexcept override;

  virtual unsigned child_count() const noexcept = 0;

  virtual Node *get_child(unsigned n) noexcept = 0;

  const Node *get_child(unsigned n) const noexcept
  { return const_cast<Tree*>(this)->get_child(n); }

#include "tree-iterator.inc"

  const_iterator cbegin() const noexcept
  { return const_iterator{this, 0}; }

  const_iterator begin() const noexcept
  { return cbegin(); }

  iterator begin() noexcept
  { return iterator{this, 0}; }

  const_iterator cend() const noexcept
  { return const_iterator{this, child_count()}; }

  const_iterator end() const noexcept
  { return cend(); }

  iterator end() noexcept
  { return iterator{this, child_count()}; }
};

// A Leaf represents one token with the surrounding trivia.
struct Leaf : Node {
  static constinit const std::string_view Tag;

  Leaf() = default;
  Leaf(const Leaf &) = delete;
  Leaf &operator=(const Leaf &) = delete;
  Leaf(Leaf &&) = default;
  Leaf &operator=(Leaf &&) = default;

  explicit Leaf(Token token)
    : _token{token}
  {}

  ~Leaf();

  std::string_view tag() const noexcept override final;

  bool isa(std::string_view tag) const noexcept override final;

  RefSpan span() const noexcept override final;

  void visit(Visitor &v) override final;

  Token &token() noexcept
  { return _token; }

  const Token &token() const noexcept
  { return _token; }

  /// @returns The Trivia node preceding this node.
  Token::TriviaList &trivia_before() noexcept
  { return _token.trivia(); }

  /// @returns The Trivia node preceding this node.
  const Token::TriviaList &trivia_before() const noexcept
  { return _token.trivia(); }

  /// @returns Pointer to the Trivia node following this node; may be null.
  Token::TriviaList *trivia_after() noexcept
  { return _trivia_after; }

  /// @returns Pointer to the Trivia node following this node; may be null.
  const Token::TriviaList *trivia_after() const noexcept
  { return _trivia_after; }

  explicit operator bool() const noexcept
  { return bool(_token); }

private:
  Token _token = {};
  Token::TriviaList *_trivia_after = nullptr;
};

// List of adjacent nodes.
struct List : Tree {
  LAVA_EXTENDS_TREE(List);

  boost::container::small_vector<NodePtr, 2> chain = {};
};

// }}}

// Expressions {{{

struct Expr : Tree {
  static constinit const std::string_view Tag;

  void visit(Visitor &v) override;

  std::string_view tag() const noexcept override;

  bool isa(std::string_view tag) const noexcept override;
};

// Expression of type '(' <expr> ')'
struct Bracketed : Expr {
  LAVA_EXTENDS_TREE(Bracketed);

  NodePtr open = {};
  NodePtr close = {};
  NodePtr expr = {};
};

// When is_postfix is false:
//   <op> <expr>
// When is_postfix is true:
//   <expr> <op>
struct Unary : Expr {
  LAVA_EXTENDS_TREE(Unary);

  bool is_postfix = false;
  NodePtr op = {};
  NodePtr expr = {};
};

// When `is_right_recursive` is false:
//   (((<first> <op0> <expr0>) <op1> <expr1>) <op2> <expr2>)...
// When `is_right_recursive` is true:
//   (<expr2> <op2> (<expr1> <op1> (<expr0> <op0> <first>)))...
// If `chain` has more than one element, all operators should be the same.
struct Infix : Expr {
  LAVA_EXTENDS_TREE(Infix);

  bool is_right_recursive = false;

  NodePtr first = {};

  // Pairs of {op, expr}.
  boost::container::small_vector<std::pair<NodePtr, NodePtr>, 1> chain = {};
};

// }}}

// Items {{{

struct ItemDecl : Tree {
  LAVA_EXTENDS_TREE(ItemDecl);

  Leaf item_word;
  Leaf first_item;

  // Pair of {comma, ident}.
  std::vector<std::pair<Leaf, Leaf>> item_list;
};

struct Namespace : Tree {
  LAVA_EXTENDS_TREE(Namespace);

  Leaf ns_word;
  Infix path;
  Leaf open_brace;
  NodePtr body;
  Leaf close_brace;
};

struct Interface : Tree {
  LAVA_EXTENDS_TREE(Interface);

  Leaf interface_word;
  Leaf name;
  Leaf open_brace;
  NodePtr body;
  Leaf close_brace;
};

struct StructUnion : Tree {
  LAVA_EXTENDS_TREE(StructUnion);

  Leaf struct_union_word;
  Leaf name;
  // TODO align
  Leaf open_brace;
  NodePtr body;
  Leaf close_brace;
};

struct Enum : Tree {
  LAVA_EXTENDS_TREE(Enum);

  Leaf enum_word;
  Leaf name;
  // TODO ': base_type'
  Leaf open_brace;
  NodePtr body;
  Leaf close_brace;
};

struct TypeAlias : Tree {
  LAVA_EXTENDS_TREE(TypeAlias);

  Leaf type_word;
  Leaf name;
  Leaf eq;
  NodePtr target;
  Leaf semi;
};

struct FunDecl : Tree {
  LAVA_EXTENDS_TREE(FunDecl);

  Leaf fun_word;
  Leaf name;
  Leaf open_paren;
  NodePtr param_list;
  Leaf close_paren;
  Leaf semi;
};

struct Fun : Tree {
  LAVA_EXTENDS_TREE(Fun);

  Leaf fun_word;
  Leaf name;
  Leaf open_paren;
  NodePtr param_list;
  Leaf close_paren;
  Leaf open_brace;
  NodePtr body;
  Leaf close_brace;
};

struct Var : Tree {
  LAVA_EXTENDS_TREE(Var);

  Leaf var_word;
  Leaf name;
  Leaf semi;
};

// }}}

// The default visitor visits every Leaf in order (DFS).
struct Visitor {
  virtual void visit_node(Node &node);
  virtual void visit_leaf(Leaf &node);
  virtual void visit_tree(Tree &node);
  virtual void visit_list(List &node);
  virtual void visit_expr(Expr &node);
  virtual void visit_bracketed(Bracketed &node);
  virtual void visit_unary(Unary &node);
  virtual void visit_infix(Infix &node);
  virtual void visit_item_decl(ItemDecl &node);
  virtual void visit_namespace(Namespace &node);
  virtual void visit_interface(Interface &node);
  virtual void visit_struct_union(StructUnion &node);
  virtual void visit_enum(Enum &node);
  virtual void visit_type_alias(TypeAlias &node);
  virtual void visit_fun_decl(FunDecl &node);
  virtual void visit_fun(Fun &node);
  virtual void visit_var(Var &node);
};

} // namespace lava::syn

#endif /* LAVA_SYN_SYNTAX_H_ */
