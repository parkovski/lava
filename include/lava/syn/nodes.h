#ifndef LAVA_SYN_NODES_H_
#define LAVA_SYN_NODES_H_

#include "token.h"
#include <vector>
#include <memory>
#include <optional>

namespace lava::syn {

enum class NodeKind {
  Document,
  Expr,
  Item,
};

struct Node {
public:
  virtual NodeKind node_kind() const = 0;
  virtual SourceLoc start() const = 0;
  virtual SourceLoc end() const = 0;
};

struct Item;
struct Document final : Node {
private:
  std::vector<std::unique_ptr<Item>> _items;

public:
  explicit Document(std::vector<std::unique_ptr<Item>> items) noexcept
    : _items{std::move(items)}
  {}

  NodeKind node_kind() const override;
  SourceLoc start() const override;
  SourceLoc end() const override;

  const std::vector<std::unique_ptr<Item>> &items() const { return _items; }
};

enum class ExprKind {
  Literal,
  Ident,
  Prefix,
  Postfix,
  Binary,
  Paren,
  Invoke,
  Scope,
};

struct Expr : Node {
  virtual NodeKind node_kind() const override;
  virtual ExprKind expr_kind() const = 0;
};

enum class LiteralType {
  Int,
  Float,
  Double,
  String,
};

struct LiteralExpr final : Expr {
private:
  LiteralType _type;
  Token _token;
  union {
    uint64_t u;
    float f;
    double d;
    std::string_view s;
  } _value;

public:
  explicit LiteralExpr(Token token, uint64_t u) noexcept
    : _type{LiteralType::Int}
    , _token{token}
    , _value{.u = u}
  {}

  explicit LiteralExpr(Token token, float f) noexcept
    : _type{LiteralType::Float}
    , _token{token}
    , _value{.f = f}
  {}

  explicit LiteralExpr(Token token, double d) noexcept
    : _type{LiteralType::Double}
    , _token{token}
    , _value{.d = d}
  {}

  explicit LiteralExpr(Token token, std::string_view s) noexcept
    : _type{LiteralType::String}
    , _token{token}
    , _value{.s = s}
  {}

  LiteralType type() const { return _type; }

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  uint64_t int_value() const { return _value.u; }
  float float_value() const { return _value.f; }
  double double_value() const { return _value.d; }
  std::string_view string_value() const { return _value.s; }
};

struct IdentExpr final : Expr {
private:
  Token _token;

public:
  explicit IdentExpr(Token token) noexcept
    : _token{token}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  std::string_view value() const { return _token.text(); }
};

struct PrefixExpr final : Expr {
private:
  Token _op;
  std::unique_ptr<Expr> _expr;

public:
  explicit PrefixExpr(Token op, std::unique_ptr<Expr> expr)
    noexcept
    : _op{op}
    , _expr{std::move(expr)}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  int op() const { return _op.what; }
  const Expr *expr() const { return _expr.get(); }
};

struct PostfixExpr final : Expr {
private:
  Token _op;
  std::unique_ptr<Expr> _expr;

public:
  explicit PostfixExpr(Token op, std::unique_ptr<Expr> expr)
    noexcept
    : _op{op}
    , _expr{std::move(expr)}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  int op() const { return _op.what; }
  const Expr *expr() const { return _expr.get(); }
};

struct BinaryExpr final : Expr {
private:
  Token _op;
  std::unique_ptr<Expr> _left;
  std::unique_ptr<Expr> _right;

public:
  explicit BinaryExpr(Token op, std::unique_ptr<Expr> left,
                      std::unique_ptr<Expr> right) noexcept
    : _op{op}
    , _left{std::move(left)}
    , _right{std::move(right)}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  int op() const { return _op.what; }
  const Expr *left() const { return _left.get(); }
  const Expr *right() const { return _right.get(); }
};

struct ParenExpr final : Expr {
private:
  Token _left;
  Token _right;
  std::unique_ptr<Expr> _expr;

public:
  explicit ParenExpr(Token left, Token right,
                     std::unique_ptr<Expr> expr) noexcept
    : _left{left}
    , _right{right}
    , _expr{std::move(expr)}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  const Expr *expr() const { return _expr.get(); }
};

template<class T>
struct WithDelimiter {
  T value;
  std::optional<Token> delimiter;

  WithDelimiter(T &&value)
    : value{std::forward<T>(value)}
    , delimiter{}
  {}

  WithDelimiter(T &&value, Token delimiter)
    : value{std::forward<T>(value)}
    , delimiter{delimiter}
  {}
};

using ExprWithDelimiter = WithDelimiter<std::unique_ptr<Expr>>;
using ExprsWithDelimiter = std::vector<ExprWithDelimiter>;

struct InvokeExpr final : Expr {
private:
  std::unique_ptr<Expr> _expr;
  Token _lparen;
  Token _rparen;
  ExprsWithDelimiter _args;

public:
  enum BracketKind {
    Paren,
    Square,
    Angle,
  };

  explicit InvokeExpr(std::unique_ptr<Expr> expr, Token lparen, Token rparen,
                      ExprsWithDelimiter args) noexcept
    : _expr{std::move(expr)}
    , _lparen{lparen}
    , _rparen{rparen}
    , _args{std::move(args)}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  const Expr *expr() const { return _expr.get(); }
  const ExprsWithDelimiter &args() const { return _args; }
  BracketKind bracket_kind() const;
};

struct ScopeExpr final : Expr {
private:
  Token _lbrace;
  Token _rbrace;
  ExprsWithDelimiter _exprs;

public:
  ScopeExpr(Token lbrace, Token rbrace, ExprsWithDelimiter exprs)
    noexcept
    : _lbrace{lbrace}
    , _rbrace{rbrace}
    , _exprs{std::move(exprs)}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  const ExprsWithDelimiter &exprs() const { return _exprs; }
};

enum class ItemKind {
  Empty,
  Expr,
  VarDecl,
  FunDecl,
  FunDef,
};

struct Item : Node {
public:
  NodeKind node_kind() const override;

  virtual ItemKind item_kind() const = 0;
};

struct EmptyItem : Item {
private:
  Token _semi;

public:
  explicit EmptyItem(Token semi) noexcept
    : _semi{semi}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ItemKind item_kind() const override;
};

struct ExprItem : Item {
private:
  std::unique_ptr<Expr> _expr;
  Token _semi;

public:
  explicit ExprItem(std::unique_ptr<Expr> expr, Token semi) noexcept
    : _expr{std::move(expr)}
    , _semi{semi}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ItemKind item_kind() const override;

  const Expr *expr() const { return _expr.get(); }
};

struct VarInit {
private:
  Token _eq;
  std::unique_ptr<Expr> _expr;

public:
  explicit VarInit(Token eq, std::unique_ptr<Expr> expr) noexcept
    : _eq{eq}
    , _expr{std::move(expr)}
  {}

  const Expr *expr() const { return _expr.get(); }
};

struct VarDecl {
private:
  Token _name;
  std::optional<VarInit> _init;

public:
  explicit VarDecl(Token name)
    : _name{name}
    , _init{}
  {}

  explicit VarDecl(Token name, VarInit init)
    : _name{name}
    , _init{std::move(init)}
  {}

  std::string_view name() const { return _name.text(); }
  const std::optional<VarInit> &init() const { return _init; }
};

using VarDeclWithDelimiter = WithDelimiter<VarDecl>;
using VarDeclsWithDelimiter = std::vector<VarDeclWithDelimiter>;

struct VarDeclItem final : Item {
private:
  std::unique_ptr<Expr> _type;
  VarDeclsWithDelimiter _decls;
  Token _semi;

public:
  explicit VarDeclItem(std::unique_ptr<Expr> type, VarDeclsWithDelimiter decls,
                       Token semi) noexcept
    : _type{std::move(type)}
    , _decls{std::move(decls)}
    , _semi{semi}
  {}

  SourceLoc start() const override;
  SourceLoc end() const override;
  ItemKind item_kind() const override;

  const Expr *type() const { return _type.get(); }
  const VarDeclsWithDelimiter &decls() const { return _decls; }
};

struct ArgDecl {
  std::unique_ptr<Expr> _type;
  Token _name;
  std::optional<VarInit> _init;

public:
  explicit ArgDecl(std::unique_ptr<Expr> type, Token name)
    : _type{std::move(type)}
    , _name{name}
    , _init{}
  {}

  explicit ArgDecl(std::unique_ptr<Expr> type, Token name, VarInit init)
    : _type{std::move(type)}
    , _name{name}
    , _init{std::move(init)}
  {}

  const Expr *type() const { return _type.get(); }
  std::string_view name() const { return _name.text(); }
  const std::optional<VarInit> &init() const { return _init; }
};

using ArgDeclWithDelimiter = WithDelimiter<ArgDecl>;
using ArgDeclsWithDelimiter = std::vector<ArgDeclWithDelimiter>;

struct ArgList {
private:
  Token _lparen;
  Token _rparen;
  ArgDeclsWithDelimiter _args;

public:
  explicit ArgList(Token lparen, Token rparen, ArgDeclsWithDelimiter args)
    noexcept
    : _lparen{lparen}
    , _rparen{rparen}
    , _args{std::move(args)}
  {}

  const ArgDeclsWithDelimiter &args() const { return _args; }
};

struct ReturnSpec {
private:
  Token _arrow;
  std::unique_ptr<Expr> _type;

public:
  explicit ReturnSpec(Token arrow, std::unique_ptr<Expr> type)
    : _arrow{arrow}
    , _type{std::move(type)}
  {}

  const Expr *type() const { return _type.get(); }
};

struct FunItemBase : Item {
private:
  Token _fun;
  Token _name;
  ArgList _args;
  std::optional<ReturnSpec> _return;

public:
  explicit FunItemBase(Token fun, Token name, ArgList args,
                       std::optional<ReturnSpec> ret) noexcept
    : _fun{fun}
    , _name{name}
    , _args{std::move(args)}
    , _return{std::move(ret)}
  {}

  SourceLoc start() const override;

  std::string_view name() const { return _name.text(); }
  const ArgDeclsWithDelimiter &args() const { return _args.args(); }
  const Expr *return_type() const
  { return _return ? _return->type() : nullptr; }
};

struct FunDeclItem final : FunItemBase {
private:
  Token _semi;

public:
  explicit FunDeclItem(Token fun, Token name, ArgList args,
                       std::optional<ReturnSpec> ret, Token semi) noexcept
    : FunItemBase{fun, name, std::move(args), std::move(ret)}
    , _semi{semi}
  {}

  SourceLoc end() const override;
  ItemKind item_kind() const override;
};

struct FunDefItem final : FunItemBase {
private:
  ScopeExpr _body;

public:
  explicit FunDefItem(Token fun, Token name, ArgList args,
                      std::optional<ReturnSpec> ret, ScopeExpr body)
    noexcept
    : FunItemBase{fun, name, std::move(args), std::move(ret)}
    , _body{std::move(body)}
  {}

  SourceLoc end() const override;
  ItemKind item_kind() const override;

  const ScopeExpr &body() const { return _body; }
};

} // namespace lava::syn

#endif // LAVA_SYN_NODES_H_
