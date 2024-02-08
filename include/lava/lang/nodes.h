#ifndef LAVA_LANG_NODES_H_
#define LAVA_LANG_NODES_H_

#include "token.h"
#include <vector>
#include <memory>
#include <optional>

namespace lava::lang {

enum class NodeKind {
  Document,
  Expr,
  Item,
};

struct Node {
public:
  virtual ~Node() = 0;
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

  Document(Document&&) = default;
  Document &operator=(Document&&) = default;

  ~Document();

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
  Return,
  If,
  While,
  Loop,
  BreakContinue,
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

  LiteralExpr(const LiteralExpr&) = default;
  LiteralExpr &operator=(const LiteralExpr&) = default;

  ~LiteralExpr();

  LiteralType type() const { return _type; }

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  const Token &token() const { return _token; }

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

  IdentExpr(const IdentExpr&) = default;
  IdentExpr &operator=(const IdentExpr &) = default;

  ~IdentExpr();

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

  PrefixExpr(PrefixExpr&&) = default;
  PrefixExpr &operator=(PrefixExpr&&) = default;

  ~PrefixExpr();

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

  PostfixExpr(PostfixExpr&&) = default;
  PostfixExpr &operator=(PostfixExpr&&) = default;

  ~PostfixExpr();

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

  BinaryExpr(BinaryExpr&&) = default;
  BinaryExpr &operator=(BinaryExpr&&) = default;

  ~BinaryExpr();

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

  ParenExpr(ParenExpr&&) = default;
  ParenExpr &operator=(ParenExpr&&) = default;

  ~ParenExpr();

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

  WithDelimiter(WithDelimiter&&) = default;
  WithDelimiter &operator=(WithDelimiter&&) = default;
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

  InvokeExpr(InvokeExpr&&) = default;
  InvokeExpr &operator=(InvokeExpr&&) = default;

  ~InvokeExpr();

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

  ScopeExpr(ScopeExpr&&) = default;
  ScopeExpr &operator=(ScopeExpr&&) = default;

  ~ScopeExpr();

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  const ExprsWithDelimiter &exprs() const { return _exprs; }
};

struct ReturnExpr final : Expr {
private:
  Token _return;
  std::unique_ptr<Expr> _expr;

public:
  ReturnExpr(Token return_, std::unique_ptr<Expr> expr)
    : _return{return_}
    , _expr{std::move(expr)}
  {}

  ReturnExpr(Token return_)
    : _return{return_}
    , _expr{}
  {}

  ~ReturnExpr();

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  const Expr *expr() const { return _expr.get(); }
};

struct ElsePart {
private:
  Token _else;
  Token _if;
  std::unique_ptr<Expr> _expr;
  ScopeExpr _scope;

public:
  ElsePart(Token else_, Token if_, std::unique_ptr<Expr> expr, ScopeExpr scope)
    : _else{else_}
    , _if{if_}
    , _expr{std::move(expr)}
    , _scope{std::move(scope)}
  {}

  ElsePart(Token else_, ScopeExpr scope)
    : _else{else_}
    , _if{}
    , _expr{}
    , _scope{std::move(scope)}
  {}

  ElsePart(ElsePart&&) = default;
  ElsePart &operator=(ElsePart&&) = default;

  SourceLoc start() const { return _else.start; }
  SourceLoc end() const { return _scope.end(); }
  const Expr *expr() const { return _expr.get(); }
  const ScopeExpr &scope() const { return _scope; }
};

struct IfExpr final : Expr {
private:
  Token _if;
  std::unique_ptr<Expr> _expr;
  ScopeExpr _scope;
  std::vector<ElsePart> _elses;

public:
  IfExpr(Token if_, std::unique_ptr<Expr> expr, ScopeExpr scope,
         std::vector<ElsePart> elses)
    : _if{if_}
    , _expr{std::move(expr)}
    , _scope{std::move(scope)}
    , _elses{std::move(elses)}
  {}

  ~IfExpr();

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  const Expr *expr() const { return _expr.get(); }
  const ScopeExpr &scope() const { return _scope; }
  const std::vector<ElsePart> &elses() const { return _elses; }
};

struct WhileExpr final : Expr {
private:
  Token _while;
  std::unique_ptr<Expr> _expr;
  ScopeExpr _scope;

public:
  WhileExpr (Token while_, std::unique_ptr<Expr> expr, ScopeExpr scope)
    : _while{while_}
    , _expr{std::move(expr)}
    , _scope{std::move(scope)}
  {}

  ~WhileExpr();

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  const Expr *expr() const { return _expr.get(); }
  const ScopeExpr &scope() const { return _scope; }
};

struct LoopExpr final : Expr {
private:
  Token _loop;
  ScopeExpr _scope;

public:
  LoopExpr(Token loop, ScopeExpr scope)
    : _loop{loop}
    , _scope{std::move(scope)}
  {}

  ~LoopExpr();

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  const ScopeExpr &scope() const { return _scope; }
};

struct BreakContinueExpr final : Expr {
private:
  Token _break_or_continue;
  std::unique_ptr<Expr> _expr;

public:
  BreakContinueExpr(Token break_or_continue, std::unique_ptr<Expr> expr)
    : _break_or_continue{break_or_continue}
    , _expr{std::move(expr)}
  {}

  BreakContinueExpr(Token break_or_continue)
    : _break_or_continue(break_or_continue)
    , _expr{nullptr}
  {}

  ~BreakContinueExpr();

  SourceLoc start() const override;
  SourceLoc end() const override;
  ExprKind expr_kind() const override;

  bool is_break() const { return _break_or_continue.what == TkBreak; }
  bool is_continue() const { return _break_or_continue.what == TkContinue; }
  const Expr *expr() const { return _expr.get(); }
};

enum class ItemKind {
  Empty,
  Expr,
  VarDecl,
  FunDecl,
  FunDef,
  StructDef,
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

  EmptyItem(const EmptyItem&) = default;
  EmptyItem &operator=(const EmptyItem&) = default;

  ~EmptyItem();

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

  ExprItem(ExprItem&&) = default;
  ExprItem &operator=(ExprItem&&) = default;

  ~ExprItem();

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

  VarInit(VarInit&&) = default;
  VarInit &operator=(VarInit&&) = default;

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

  VarDecl(VarDecl&&) = default;
  VarDecl &operator=(VarDecl&&) = default;

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

  VarDeclItem(VarDeclItem&&) = default;
  VarDeclItem &operator=(VarDeclItem&&) = default;

  ~VarDeclItem();

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

  ArgDecl(ArgDecl&&) = default;
  ArgDecl &operator=(ArgDecl&&) = default;

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

  ArgList(ArgList&&) = default;
  ArgList &operator=(ArgList&&) = default;

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

  FunItemBase(FunItemBase&&) = default;
  FunItemBase &operator=(FunItemBase&&) = default;

  ~FunItemBase();

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

  FunDeclItem(FunDeclItem&&) = default;
  FunDeclItem &operator=(FunDeclItem&&) = default;

  ~FunDeclItem();

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

  FunDefItem(FunDefItem&&) = default;
  FunDefItem &operator=(FunDefItem&&) = default;

  ~FunDefItem();

  SourceLoc end() const override;
  ItemKind item_kind() const override;

  const ScopeExpr &body() const { return _body; }
};

struct StructDefItem final : Item {
private:
  Token _struct_or_union;
  Token _name;
  Token _lbrace;
  Token _rbrace;
  std::vector<VarDeclItem> _vars;

public:
  explicit StructDefItem(Token struct_or_union, Token name, Token lbrace,
                         Token rbrace, std::vector<VarDeclItem> vars)
    noexcept
    : _struct_or_union{struct_or_union}
    , _name{name}
    , _lbrace{lbrace}
    , _rbrace{rbrace}
    , _vars{std::move(vars)}
  {}

  StructDefItem(StructDefItem&&) = default;
  StructDefItem &operator=(StructDefItem&&) = default;

  ~StructDefItem();

  SourceLoc start() const override;
  SourceLoc end() const override;
  ItemKind item_kind() const override;

  bool is_union() const {
    return _struct_or_union.what == TkUnion;
  }

  std::string_view name() const { return _name.text(); }
  const std::vector<VarDeclItem> &vars() const { return _vars; }
};

} // namespace lava::lang

#endif // LAVA_LANG_NODES_H_
