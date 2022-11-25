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

// Base classes {{{

// Syntax tree base type.
struct Node {
  // Type info for subclasses of Node.
  enum class Type {
    Leaf,
    Bracketed,
    Unary,
    Infix,
    Adjacent,
  };

  virtual ~Node() = 0;

  // The root node is the only node with no parent.
  bool is_root() const noexcept
  { return _parent == nullptr; }

  void set_parent(Tree *parent) noexcept
  { _parent = parent; }

  // Get this node's parent node. Always defined unless `is_root()` is true.
  Tree *parent() noexcept
  { return _parent; }

  // Get this node's parent node. Always defined unless `is_root()` is true.
  const Tree *parent() const noexcept
  { return _parent; }

  void set_tag(const char *tag) noexcept
  { _tag = tag; }

  const char *tag() const noexcept
  { return _tag; }

  // Total source span of all the tokens in this node.
  virtual RefSpan span() const noexcept = 0;

  // Runtime type info.
  virtual Type type() const noexcept = 0;

  // Visit this node.
  virtual void visit(Visitor &v) = 0;

private:
  const char *_tag = nullptr;
  // Parent node; only the root node may be null.
  Tree *_parent = nullptr;
};

struct Tree : virtual Node {
  virtual unsigned child_count() const noexcept = 0;

  virtual Node *get_child(unsigned n) noexcept = 0;

  const Node *get_child(unsigned n) const noexcept
  { return const_cast<Tree*>(this)->get_child(n); }

  struct const_iterator {
    typedef void difference_type;
    typedef const Node &value_type;
    typedef const Node *pointer;
    typedef const Node &reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    explicit const_iterator(const Tree *parent, unsigned index)
             noexcept
      : _parent{const_cast<Tree*>(parent)}
      , _index{index}
    {}

    const_iterator(const const_iterator &) = default;
    const_iterator &operator=(const const_iterator &) = default;

    friend bool operator==(const const_iterator &a, const const_iterator &b)
                = default;
    friend auto operator<=>(const const_iterator &a, const const_iterator &b)
                = default;

    friend void swap(const_iterator &a, const_iterator &b) noexcept {
      using std::swap;
      swap(a._parent, b._parent);
      swap(a._index, b._index);
    }

    const Node &operator*() const {
      return *_parent->get_child(_index);
    }

    const Node &operator->() const {
      return **this;
    }

    const_iterator &operator++() {
      ++_index;
      return *this;
    }

    const_iterator operator++(int) {
      const_iterator pre = *this;
      ++_index;
      return pre;
    }

    const_iterator &operator--() {
      --_index;
      return *this;
    }

    const_iterator operator--(int) {
      const_iterator pre = *this;
      --_index;
      return pre;
    }

  protected:
    Tree *_parent = nullptr;
    unsigned _index = 0;
  };

  struct iterator : const_iterator {
    using const_iterator::difference_type;
    typedef Node &value_type;
    typedef Node *pointer;
    typedef Node &reference;
    using const_iterator::iterator_category;

    explicit iterator(Tree *parent, unsigned index) noexcept
      : const_iterator{parent, index}
    {}

    iterator(const iterator &) = default;
    iterator &operator=(const iterator &) = default;

    friend bool operator==(const iterator &a, const iterator &b) = default;
    friend auto operator<=>(const iterator &a, const iterator &b) = default;

    friend void swap(iterator &a, iterator &b) {
      swap(static_cast<const_iterator&>(a), static_cast<const_iterator&>(b));
    }

    Node &operator*() {
      return *_parent->get_child(_index);
    }

    Node &operator->() {
      return **this;
    }

    iterator &operator++() {
      ++_index;
      return *this;
    }

    iterator operator++(int) {
      iterator pre = *this;
      ++_index;
      return pre;
    }

    iterator &operator--() {
      --_index;
      return *this;
    }

    iterator operator--(int) {
      iterator pre = *this;
      --_index;
      return pre;
    }
  };

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
  explicit Leaf(Token token)
    : _token{token}
  {}

  ~Leaf();

  RefSpan span() const noexcept override final;
  Type type() const noexcept override final;
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

// }}}

// Expression of kind <expr0> <expr1>...
struct Adjacent : Tree {
  ~Adjacent();

  RefSpan span() const noexcept override final;

  Type type() const noexcept override final;
  void visit(Visitor &v) override final;

  unsigned child_count() const noexcept override final;
  Node *get_child(unsigned n) noexcept override final;

  boost::container::small_vector<NodePtr, 2> chain = {};
};

// Expression of type '(' <expr> ')'
struct Bracketed : Tree {
  ~Bracketed();

  RefSpan span() const noexcept override final;

  Type type() const noexcept override final;
  void visit(Visitor &v) override final;

  unsigned child_count() const noexcept override final;
  Node *get_child(unsigned n) noexcept override final;

  NodePtr open = {};
  NodePtr close = {};
  NodePtr expr = {};
};

// When is_postfix is false:
//   <op> <expr>
// When is_postfix is true:
//   <expr> <op>
struct Unary : Tree {
  ~Unary();

  RefSpan span() const noexcept override final;

  Type type() const noexcept override final;
  void visit(Visitor &v) override final;

  unsigned child_count() const noexcept override final;
  Node *get_child(unsigned n) noexcept override final;

  bool is_postfix = false;
  NodePtr op = {};
  NodePtr expr = {};
};

// When `is_right_recursive` is false:
//   (((<first> <op0> <expr0>) <op1> <expr1>) <op2> <expr2>)...
// When `is_right_recursive` is true:
//   (<expr2> <op2> (<expr1> <op1> (<expr0> <op0> <first>)))...
struct Infix : Tree {
  ~Infix();

  RefSpan span() const noexcept override final;

  Type type() const noexcept override final;
  void visit(Visitor &v) override final;

  unsigned child_count() const noexcept override final;
  Node *get_child(unsigned n) noexcept override final;

  bool is_right_recursive = false;

  NodePtr first = {};

  // Pairs of {op, expr}.
  boost::container::small_vector<std::pair<NodePtr, NodePtr>, 1> chain = {};
};

struct Visitor {
  virtual void visit_leaf(Leaf &node);
  virtual void visit_adjacent(Adjacent &node);
  virtual void visit_bracketed(Bracketed &node);
  virtual void visit_unary(Unary &node);
  virtual void visit_infix(Infix &node);
};

} // namespace lava::syn

#endif /* LAVA_SYN_SYNTAX_H_ */
