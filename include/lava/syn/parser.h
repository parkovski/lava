#ifndef LAVA_SYN_PARSER_H_
#define LAVA_SYN_PARSER_H_

#include "lexer.h"
#include <memory>
#include <vector>
#include <optional>

namespace lava::syn {

struct Node {
  const SourceDoc *doc;
  SourceLoc start;
  SourceLoc end;

  explicit Node(const SourceDoc *doc, SourceLoc start, SourceLoc end) noexcept
    : doc{doc}
    , start{start}
    , end{end}
  {}
};

struct Expr : Node {
  enum Kind {
    Literal,
    Ident,
    Prefix,
    Postfix,
    Binary,
    Paren,
    Invoke,
  } kind;

  explicit Expr(Kind kind, const SourceDoc *doc, SourceLoc start,
                SourceLoc end) noexcept
    : Node{doc, start, end}
    , kind{kind}
  {}
};

struct LiteralExpr : Expr {
  enum Type {
    Int,
    Float,
    Double,
    String,
  } type;

  union {
    uint64_t u;
    float f;
    double d;
    std::string_view str;
  } value;

  explicit LiteralExpr(Token token, uint64_t u) noexcept
    : Expr{Literal, token.doc, token.start, token.end}
    , type{Int}
    , value{.u = u}
  {}

  explicit LiteralExpr(Token token, float f) noexcept
    : Expr{Literal, token.doc, token.start, token.end}
    , type{Float}
    , value{.f = f}
  {}

  explicit LiteralExpr(Token token, double d) noexcept
    : Expr{Literal, token.doc, token.start, token.end}
    , type{Double}
    , value{.d = d}
  {}

  explicit LiteralExpr(Token token, std::string_view str) noexcept
    : Expr{Literal, token.doc, token.start, token.end}
    , type{String}
    , value{.str = str}
  {}
};

struct IdentExpr : Expr {
  std::string_view value;

  explicit IdentExpr(Token token) noexcept
    : Expr{Ident, token.doc, token.start, token.end}
    , value{token.text()}
  {}
};

struct PrefixExpr : Expr {
  Token op;
  std::unique_ptr<Expr> expr;

  explicit PrefixExpr(Token op, std::unique_ptr<Expr> expr) noexcept
    : Expr{Prefix, op.doc, op.start, expr->end}
    , op{op}
    , expr{std::move(expr)}
  {}
};

struct PostfixExpr : Expr {
  Token op;
  std::unique_ptr<Expr> expr;

  explicit PostfixExpr(Token op, std::unique_ptr<Expr> expr) noexcept
    : Expr{Postfix, op.doc, expr->start, op.end}
    , op{op}
    , expr{std::move(expr)}
  {}
};

struct BinaryExpr : Expr {
  Token op;
  std::unique_ptr<Expr> left;
  std::unique_ptr<Expr> right;

  explicit BinaryExpr(Token op, std::unique_ptr<Expr> left,
                      std::unique_ptr<Expr> right) noexcept
    : Expr{Binary, op.doc, left->start, right->end}
    , op{op}
    , left{std::move(left)}
    , right{std::move(right)}
  {}
};

struct ParenExpr : Expr {
  Token left;
  Token right;
  std::unique_ptr<Expr> expr;

  explicit ParenExpr(Token left, Token right, std::unique_ptr<Expr> expr)
    noexcept
    : Expr{Paren, left.doc, left.start, right.end}
    , left{left}
    , right{right}
    , expr{std::move(expr)}
  {}
};

struct ExprWithDelimiter {
  std::unique_ptr<Expr> expr;
  std::optional<Token> delimiter;
};

struct InvokeExpr : Expr {
  std::unique_ptr<Expr> left;
  Token lparen;
  Token rparen;
  std::vector<ExprWithDelimiter> args;

  explicit InvokeExpr(std::unique_ptr<Expr> left, Token lparen, Token rparen,
                      std::vector<ExprWithDelimiter> args) noexcept
    : Expr{Invoke, lparen.doc, left->start, rparen.end}
    , left{std::move(left)}
    , lparen{lparen}
    , rparen{rparen}
    , args{std::move(args)}
  {}
};

struct Parser {
private:
  Lexer *lexer;
  Token token;

public:
  enum Flags {
    PF_NoComma = 1
  };

  explicit Parser(Lexer &lexer) noexcept;

  std::unique_ptr<Expr> parse_expr(unsigned prec = 1, int flags = 0);

private:
  void next();

  std::unique_ptr<InvokeExpr> parse_invoke_expr(std::unique_ptr<Expr> left);
  std::unique_ptr<Expr> parse_primary();

  static unsigned get_prefix_prec(int op, int flags);
  static unsigned get_infix_prec(int op, int flags);
  static unsigned get_postfix_prec(int op, int flags);
};

} // namespace lava::syn

#endif // LAVA_SYN_PARSER_H_
