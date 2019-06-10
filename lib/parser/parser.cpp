#include "ash/parser/parser.h"

#include <new>

using namespace ash;

SyntaxNode::~SyntaxNode() {
}

UnaryExpressionNode::~UnaryExpressionNode() {
}

BinaryExpressionNode::~BinaryExpressionNode() {
}

TernaryExpressionNode::~TernaryExpressionNode() {
}

TerminalNode::~TerminalNode() {
}

std::unique_ptr<CallNode>
CallNode::create(std::unique_ptr<ExpressionNode> function, size_t argc,
                 std::unique_ptr<ExpressionNode> *argv,
                 size_t start, size_t end) noexcept
{
  auto mem = ::operator new(sizeof(CallNode)
                            + argc * sizeof(std::unique_ptr<ExpressionNode>));
  auto call = reinterpret_cast<CallNode *>(mem);
  new (call) CallNode(start, end);
  call->function = std::move(function);
  call->argc = argc;
  for (size_t i = 0; i < argc; ++i) {
    call->argv[i] = std::move(argv[i]);
  }
  return std::unique_ptr<CallNode>(call);
}

CallNode::~CallNode() {
  for (size_t i = 0; i < argc; ++i) {
    argv[i].~unique_ptr();
  }
}

Parser::Parser() noexcept
  : _cur(), _next()
{}

std::unique_ptr<SyntaxNode> Parser::parse() {
  return {};
}