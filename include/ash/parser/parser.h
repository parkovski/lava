#ifndef ASH_PARSER_PARSER_H_
#define ASH_PARSER_PARSER_H_

#include "lexer.h"
#include "keywords.h"
#include "ash/source/session.h"
#include "ash/ast/node.h"

namespace ash::parser {

class Visitor {
public:
  virtual void visitWhitespace() = 0;
  virtual void visitComment() = 0;
  virtual void visitLiteral() = 0;
  virtual void visitIdentifier() = 0;
};

class Parser {
public:
  explicit Parser(source::Session *session);

  enum ExprFlags : unsigned {
    // Last byte is reserved for operator precedence.
    EF_PrecMask   = 0x000000FF,
    EF_FlagMask   = 0xFFFFFF00,
    // Right-to-left parsing, only applies to infix/binary operators.
    EF_RTL        = 0x80000000,
    // Comma not allowed
    EF_NoComma    = 0x40000000,
  };

  ast::SeparatedNodeList parseExpressionListV();
  ast::NodeList parseExpressionListH();

  ast::NodePtr parseExpression(unsigned flags = 0);

  ast::NodePtr parsePrimary();
  std::unique_ptr<ast::Delimited> parseDelimited(Tk closeDelim,
                                                 unsigned flags = 0);
  std::unique_ptr<ast::Terminal> parseTerminal();
  std::unique_ptr<ast::String> parseString();

private:
  unsigned prefixPrecedence(unsigned flags) const;
  std::pair<unsigned, unsigned> infixPostfixPrecedence(unsigned flags) const;

  void collectTrivia();
  void fwd();

  template<typename TMsg, typename... Args>
  void error(source::LocId loc, TMsg &&msg, Args &&...args);

  source::Session *_session;
  Lexer _lexer;
  Token _token;
  ast::Trivia _trivia;
  KeywordMap _keywords;
  std::optional<Kw> _kw;
};

} // namespace ash::parser

#endif // ASH_PARSER_PARSER_H_
