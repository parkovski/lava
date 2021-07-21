#ifndef ASH_AST_NODE_H_
#define ASH_AST_NODE_H_

#include "ash/source/location.h"
#include "ash/data/polyvector.h"

#include <boost/container/small_vector.hpp>

#include <vector>
#include <memory>
#include <iosfwd>

namespace ash::source {
  class Session;
}

namespace ash::ast {

namespace detail {

  template<typename T>
  using vector = std::vector<T>;

  template<typename T>
  using unique_ptr = std::unique_ptr<T>;

  template<size_t N>
  struct SmallVectorWrapper {
    template<typename T>
    using type = boost::container::small_vector<T, N>;
  };

} // namespace detail

class Visitor;

class Node {
public:
  Node() = default;

  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;

protected:
  Node(Node &&) = default;
  Node &operator=(Node &&) = default;

public:
  virtual ~Node() = 0;

  // virtual source::SpanRef triviaBefore() const = 0;
  // virtual source::SpanRef triviaAfter() const = 0;
  virtual source::SpanRef span() const = 0;

  virtual void visit(Visitor &v) const = 0;
};

using NodePtr = std::unique_ptr<Node>;
using ConstNodePtr = std::unique_ptr<const Node>;

// More or less, a polymorphic vector of nodes.
using NodeVec = data::PolyVector<Node, detail::vector, detail::unique_ptr>;

// Polymorphic vector of nodes with N inline elements.
template<size_t N>
using SmallNodeVec = data::PolyVector<
  Node,
  detail::SmallVectorWrapper<N>::template type,
  detail::unique_ptr
>;

class Trivia final : public Node {
public:
  explicit Trivia() noexcept
    : _span{}
  {}

  explicit Trivia(source::SpanRef span) noexcept
    : _span{span}
  {
    assert(_span.first.isValid() && _span.second.isValid());
  }

  Trivia(Trivia &&) = default;
  Trivia &operator=(Trivia &&) = default;

  ~Trivia();

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

  bool empty() const {
    return !_span.first.isValid();
  }

private:
  source::SpanRef _span;
};

class NodeList final : public Node {
public:
  explicit NodeList(SmallNodeVec<1> &&nodes) noexcept
    : _nodes{std::move(nodes)}
  {}

  NodeList(NodeList &&) = default;
  NodeList &operator=(NodeList &&) = default;

  ~NodeList();

  SmallNodeVec<1> &nodes() { return _nodes; }
  const SmallNodeVec<1> &nodes() const { return _nodes; }

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

private:
  SmallNodeVec<1> _nodes;
};

class Terminal : public Node {
public:
  explicit Terminal(source::LocId location, Trivia &&trivia) noexcept
    : _location{location}, _trivia{std::move(trivia)}
  {}

  Terminal(Terminal &&) = default;
  Terminal &operator=(Terminal &&) = default;

  ~Terminal();

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

  const Trivia &trivia() const { return _trivia; }

private:
  source::LocId _location;
  Trivia _trivia;
};

class Separator final : public Node {
public:
  explicit Separator() noexcept
    : _separators{}
  {}

  explicit Separator(
    boost::container::small_vector<Terminal, 1> &&separators
  ) noexcept
    : _separators{std::move(separators)}
  {}

  Separator(Separator &&) = default;
  Separator &operator=(Separator &&) = default;

  ~Separator();

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

  bool empty() const {
    return _separators.empty();
  }

private:
  boost::container::small_vector<Terminal, 1> _separators;
};

class SeparatedNodeList final : public Node {
public:
  explicit SeparatedNodeList(
    Separator &&firstSeparator,
    std::vector<std::pair<NodeList, Separator>> &&lists
  ) noexcept
    : _firstSeparator{std::move(firstSeparator)},
      _lists{std::move(lists)}
  {}

  SeparatedNodeList(SeparatedNodeList &&) = default;
  SeparatedNodeList &operator=(SeparatedNodeList &&) = default;

  ~SeparatedNodeList();

  std::vector<std::pair<NodeList, Separator>> &lists() { return _lists; }
  const std::vector<std::pair<NodeList, Separator>> &lists() const {
    return _lists;
  }

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

private:
  Separator _firstSeparator;
  std::vector<std::pair<NodeList, Separator>> _lists;
};

class Literal : public Terminal {
public:
  enum class Kind : uint8_t {
    Integer,
    Float,
    String,
  };

  explicit Literal(Kind kind, source::LocId location, Trivia &&trivia) noexcept
    : Terminal(location, std::move(trivia)), _kind{kind}
  {}

  Literal(Literal &&) = default;
  Literal &operator=(Literal &&) = default;

  ~Literal();

  Kind kind() const { return _kind; }

private:
  Kind _kind;
};

class StringLiteral : public Literal {
public:
  explicit StringLiteral(source::LocId loc, source::LocId endLoc,
                         Trivia &&trivia) noexcept
    : Literal{Kind::String, loc, std::move(trivia)}, _endLoc{endLoc}
  {}

  StringLiteral(StringLiteral &&) = default;
  StringLiteral &operator=(StringLiteral &&) = default;

  ~StringLiteral();

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

private:
  source::LocId _endLoc;
};

class Identifier final : public Terminal {
public:
  using Terminal::Terminal;

  ~Identifier();
};

class Variable final : public Terminal {
public:
  using Terminal::Terminal;

  ~Variable();
};

class Operator final : public Terminal {
public:
  explicit Operator(source::LocId loc, Trivia &&trivia, unsigned precedence)
    noexcept
    : Terminal{loc, std::move(trivia)}, _precedence{precedence}
  {}

  Operator(Operator &&) = default;
  Operator &operator=(Operator &&) = default;

  ~Operator();

  unsigned precedence() const { return _precedence; }

  void visit(Visitor &v) const override;

private:
  unsigned _precedence;
};

class String final : public Node {
public:
  explicit String(source::LocId openQuote, source::LocId closeQuote,
                  SmallNodeVec<1> &&pieces, Trivia &&trivia) noexcept
    : _trivia{std::move(trivia)},
      _openQuote{openQuote},
      _closeQuote{closeQuote},
      _pieces{std::move(pieces)}
  {}

  String(String &&) = default;
  String &operator=(String &&) = default;

  ~String();

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

private:
  Trivia _trivia;
  source::LocId _openQuote;
  source::LocId _closeQuote;
  SmallNodeVec<1> _pieces;
};

class Unary final : public Node {
public:
  explicit Unary(Operator &&op, NodePtr &&expr) noexcept
    : _op{std::move(op)}, _expr{std::move(expr)}
  {}

  Unary(Unary &&) = default;
  Unary &operator=(Unary &&) = default;

  ~Unary();

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

private:
  Operator _op;
  NodePtr _expr;
};

class Binary final : public Node {
public:
  explicit Binary(Operator &&op, NodePtr &&left, NodePtr &&right) noexcept
    : _op{std::move(op)}, _left{std::move(left)}, _right{std::move(right)}
  {}

  Binary(Binary &&) = default;
  Binary &operator=(Binary &&) = default;

  ~Binary();

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

private:
  Operator _op;
  NodePtr _left;
  NodePtr _right;
};

class Delimited final : public Node {
public:
  explicit Delimited(Terminal &&openDelim, Terminal &&closeDelim,
                     SmallNodeVec<1> &&exprs) noexcept
    : _openDelim{std::move(openDelim)}, _closeDelim{std::move(closeDelim)},
      _exprs{std::move(exprs)}
  {}

  Delimited(Delimited &&) = default;
  Delimited &operator=(Delimited &&) = default;

  ~Delimited();

  source::SpanRef span() const override;

  void visit(Visitor &v) const override;

private:
  Terminal _openDelim;
  Terminal _closeDelim;
  SmallNodeVec<1> _exprs;
};

} // namespace ash::ast

#endif // ASH_AST_NODE_H_
