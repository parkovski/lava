#ifndef ASH_PARSER_H_
#define ASH_PARSER_H_

#include "lexer.h"

#include <memory>

namespace ash {

enum class SyntaxKind {
  Unknown = 0,
  Terminal = 1,
  UnaryExpression = 2,
  BinaryExpression = 3,
  TernaryExpression = 4,
  CallNode = 5,
};

struct SyntaxNode {
  SyntaxKind kind;
  size_t start;
  size_t end;

  explicit SyntaxNode(SyntaxKind kind, size_t start = 0, size_t end = 0)
    noexcept
    : kind(kind), start(start), end(end)
  {}

  virtual ~SyntaxNode() = 0;
};

struct ExpressionNode : SyntaxNode {
  using SyntaxNode::SyntaxNode;
};

struct UnaryExpressionNode final : ExpressionNode {
  Token oper;
  std::unique_ptr<ExpressionNode> expr;

  explicit UnaryExpressionNode(Token oper, std::unique_ptr<ExpressionNode> expr,
                               size_t start = 0, size_t end = 0) noexcept
    : ExpressionNode(SyntaxKind::UnaryExpression, start, end),
      oper(oper), expr(std::move(expr))
  {}

  ~UnaryExpressionNode();
};

struct BinaryExpressionNode final : ExpressionNode {
  Token oper;
  std::unique_ptr<ExpressionNode> left;
  std::unique_ptr<ExpressionNode> right;

  explicit BinaryExpressionNode(Token oper,
                                std::unique_ptr<ExpressionNode> left,
                                std::unique_ptr<ExpressionNode> right,
                                size_t start = 0, size_t end = 0) noexcept
    : ExpressionNode(SyntaxKind::BinaryExpression, start, end),
      oper(oper), left(std::move(left)), right(std::move(right))
  {}

  ~BinaryExpressionNode();
};

struct TernaryExpressionNode final : ExpressionNode {
  Token oper1;
  Token oper2;
  std::unique_ptr<ExpressionNode> left;
  std::unique_ptr<ExpressionNode> center;
  std::unique_ptr<ExpressionNode> right;

  explicit TernaryExpressionNode(Token oper1, Token oper2,
                                 std::unique_ptr<ExpressionNode> left,
                                 std::unique_ptr<ExpressionNode> center,
                                 std::unique_ptr<ExpressionNode> right,
                                 size_t start = 0, size_t end = 0) noexcept
    : ExpressionNode(SyntaxKind::TernaryExpression, start, end),
      oper1(oper1), oper2(oper2), left(std::move(left)),
      center(std::move(center)), right(std::move(right))
  {}

  ~TernaryExpressionNode();
};

struct TerminalNode final : ExpressionNode {
  Token token;

  explicit TerminalNode(Token token, size_t start = 0, size_t end = 0) noexcept
    : ExpressionNode(SyntaxKind::Terminal, start, end),
      token(token)
  {}

  ~TerminalNode();
};

struct CallNode final : ExpressionNode {
  std::unique_ptr<ExpressionNode> function;
  size_t argc;
  std::unique_ptr<ExpressionNode> argv[];

  // Variable sized struct, so use this allocation function instead of
  // the constructor.
  static std::unique_ptr<CallNode>
  create(std::unique_ptr<ExpressionNode> function, size_t argc,
         std::unique_ptr<ExpressionNode> *argv,
         size_t start = 0, size_t end = 0) noexcept;

  ~CallNode();

private:
  explicit CallNode(size_t start, size_t end) noexcept
    : ExpressionNode(SyntaxKind::CallNode, start, end)
  {}
};

class Parser {
public:
  Parser() noexcept;

  std::unique_ptr<SyntaxNode> parse();

private:
  TerminalNode parseTerminal();
  void parseId();
  void parseCall();

  Token _cur;
  Token _next;
};

}

#endif // ASH_PARSER_H_