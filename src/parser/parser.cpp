#include "ash/parser/parser.h"
#include <fmt/format.h>
#include <fmt/ostream.h>

using namespace ash;
using namespace ash::parser;

Parser::Parser(source::Session *session)
  : _session{session},
    _lexer{session},
    _token{},
    _kw{std::nullopt}
{
  fwd();
}

ast::SeparatedNodeList Parser::parseExpressionListV() {
  std::vector<std::pair<ast::NodeList, ast::Separator>> lists;

  auto collectSeparators = [this]() {
    if (_token.id == Tk::NewLine || _token.id == Tk::Semicolon) {
      boost::container::small_vector<ast::Terminal, 1> separators;
      do {
        separators.emplace_back(ast::Terminal(_token.loc, std::move(_trivia)));
        fwd();
      } while (_token.id == Tk::NewLine || _token.id == Tk::Semicolon);
      return ast::Separator{std::move(separators)};
    }
    return ast::Separator{};
  };

  auto firstSeparator = collectSeparators();

  while (true) {
    auto list = parseExpressionListH();
    if (list.nodes().empty()) {
      // ERROR
      error(_token.loc, "empty node list");
      fwd();
    } else {
      if (_token.id == Tk::EndOfInput) {
        break;
      }
      auto sep = collectSeparators();
      if (sep.empty()) {
        // ERROR
        error(_token.loc, "no separator");
      }
      lists.emplace_back(std::make_pair(std::move(list), std::move(sep)));
    }
  }

  return ast::SeparatedNodeList{std::move(firstSeparator), std::move(lists)};
}

ast::NodeList Parser::parseExpressionListH() {
  ast::SmallNodeVec<1> nodes;

  while (auto expr = parseExpression()) {
    nodes.emplace_back(std::move(expr));
  }

  return ast::NodeList{std::move(nodes)};
}

ast::NodePtr Parser::parseExpression(unsigned flags) {
  unsigned basePrec = flags & EF_PrecMask;
  if (basePrec == 0) {
    basePrec = 1;
  }
  flags &= EF_FlagMask;
  ast::NodePtr expr;

  // Parse prefix operators (always right recursive).
  if (auto prefixPrec = prefixPrecedence(flags)) {
    assert((prefixPrec & EF_FlagMask) == 0);
    auto op = ast::Operator(_token.loc, std::move(_trivia), prefixPrec);
    fwd();
    expr = parseExpression(prefixPrec | flags);
    if (!expr) {
      // ERROR
      error(_token.loc, "no expression after prefix op");
      return nullptr;
    }
    expr = std::make_unique<ast::Unary>(std::move(op), std::move(expr));
  } else {
    expr = parsePrimary();
    if (!expr) {
      // Ok
      return nullptr;
    }
  }

  // Parse LTR operators. This includes all postfix operators.
  while (true) {
    auto [infixPrec, postfixPrec] = infixPostfixPrecedence(flags);
    assert((postfixPrec & EF_FlagMask) == 0);
    auto infixFlags = infixPrec & EF_FlagMask;
    infixPrec &= EF_PrecMask;

    // Two possibilities:
    // - Infix operator with precedence >= basePrec:
    //   Parse the operator and the right side expression, then parse
    //   right-recursive (greater precedence) expressions. Combine the
    //   left (expr) and right (right) expressions.
    // - Postfix operator with precedence >= basePrec:
    //   Parse the operator and combine it with the left side (expr).
    //   There is no need to look for right recursion specifically.
    // - Both possibilities:
    //   If a valid expression follows the operator, it is considered
    //   infix. Postfix meaning can be forced with parenthesis, but
    //   infix cannot.
    auto loc = _token.loc;
    if (infixPrec >= basePrec) {
      // If this is an RTL operator, recursively parse expressions at
      // the same precedence level, otherwise only parse higher precedence
      // expressions.
      auto trivia = std::move(_trivia);
      auto rightPrec = infixPrec;
      if ((infixFlags & EF_RTL) == 0) {
        ++rightPrec;
      }
      fwd();
      auto right = parseExpression(rightPrec | flags);
      if (right) {
        expr = std::make_unique<ast::Binary>(
          ast::Operator(loc, std::move(trivia), infixPrec),
          std::move(expr),
          std::move(right)
        );
      } else if (postfixPrec >= basePrec) {
        expr = std::make_unique<ast::Unary>(
          ast::Operator(loc, std::move(trivia), postfixPrec),
          std::move(expr)
        );
      } else {
        // ERROR
        error(_token.loc, "binary op without right side");
        break;
      }
    } else if (postfixPrec >= basePrec) {
      expr = std::make_unique<ast::Unary>(
        ast::Operator(loc, std::move(_trivia), postfixPrec),
        std::move(expr)
      );
      fwd();
    } else {
      break;
    }
  }

  return expr;
}

ast::NodePtr Parser::parsePrimary() {
  switch (_token.id) {
    case Tk::LeftParen:
      return parseDelimited(Tk::RightParen);

    case Tk::LeftSquareBracket:
      return parseDelimited(Tk::RightSquareBracket);

    case Tk::LeftBrace:
      return parseDelimited(Tk::RightBrace);

    default:
      if (auto str = parseString()) {
        return str;
      }
      return parseTerminal();
  }
}

std::unique_ptr<ast::Delimited>
Parser::parseDelimited(Tk closeDelim, unsigned flags) {
  assert((flags & ~EF_NoComma) == 0 && "Invalid flags");

  ast::Terminal open(_token.loc, std::move(_trivia));
  fwd();

  ast::SmallNodeVec<1> exprs;
  while (_token.id != closeDelim) {
    if (_token.id == Tk::EndOfInput) {
      // ERROR
      error(_token.loc, "missing {}", closeDelim);
      break;
    }
    if (_token.id == Tk::Comma && (flags & EF_NoComma) == 0) {
      exprs.emplace_back(std::make_unique<ast::Operator>(
        _token.loc, std::move(_trivia), 0
      ));
      fwd();
    }
    // If the NoComma flag was specified, they are allowed as a regular
    // part of the expression instead of a special delimited node.
    if (auto expr = parseExpression(flags ^ EF_NoComma)) {
      exprs.emplace_back(std::move(expr));
    }
  }

  ast::Terminal close(_token.loc, std::move(_trivia));
  fwd();

  return std::make_unique<ast::Delimited>(
    std::move(open), std::move(close), std::move(exprs)
  );
}

std::unique_ptr<ast::Terminal> Parser::parseTerminal() {
  using LK = ast::Literal::Kind;
  std::unique_ptr<ast::Terminal> terminal;
  switch (_token.id) {
    case Tk::IntLit:
    case Tk::HexLit:
    case Tk::BinLit:
      terminal = std::make_unique<ast::Literal>(
        LK::Integer, _token.loc, std::move(_trivia)
      );
      break;

    case Tk::FloatLit:
      terminal = std::make_unique<ast::Literal>(
        LK::Float, _token.loc, std::move(_trivia)
      );
      break;

    case Tk::Ident:
      terminal = std::make_unique<ast::Identifier>(
        _token.loc, std::move(_trivia)
      );
      break;

    case Tk::Variable:
      terminal = std::make_unique<ast::Variable>(
        _token.loc, std::move(_trivia)
      );
      break;

    default:
      return nullptr;
  }

  fwd();
  return terminal;
}

std::unique_ptr<ast::String> Parser::parseString() {
  ast::Trivia trivia;
  Tk quote = _token.id;
  source::LocId openLoc = _token.loc;
  source::LocId closeLoc;
  ast::SmallNodeVec<1> pieces;

  switch (_token.id) {
    case Tk::SingleQuote:
      trivia = std::move(_trivia);
      fwd();
      if (_token.id == Tk::Text) {
        pieces.emplace_back(std::make_unique<ast::Literal>(
          ast::Literal::Kind::String, _token.loc, ast::Trivia{}
        ));
        fwd();
      }
      if (_token.id != Tk::SingleQuote) {
        // ERROR
        error(_token.loc, "missing SingleQuote");
      }
      closeLoc = _token.loc;
      fwd();
      return std::make_unique<ast::String>(
        openLoc, closeLoc, std::move(pieces), std::move(trivia)
      );

    case Tk::DoubleQuote:
    case Tk::Backtick:
      break;

    default:
      return nullptr;
  }

  trivia = std::move(_trivia);
  fwd();
  while (true) {
    if (_token.id == quote) {
      closeLoc = _token.loc;
      fwd();
      return std::make_unique<ast::String>(
        openLoc, closeLoc, std::move(pieces), std::move(trivia)
      );
    } else if (_token.id == Tk::EndOfInput) {
      // ERROR
      error(_token.loc, "missing {}", quote);
      closeLoc = _token.loc;
      return std::make_unique<ast::String>(
        openLoc, closeLoc, std::move(pieces), std::move(trivia)
      );
    }

    switch (_token.id) {
      default:
        // ERROR
        error(_token.loc, "unsupported string part {}", _token.id);
        fwd();
        break;

      case Tk::Escape:
        fwd();
        // TODO
        break;

      case Tk::DollarLeftParen:
        if (auto expr = parseDelimited(Tk::RightParen, EF_NoComma)) {
          pieces.emplace_back(std::move(expr));
        } else {
          // ERROR
          error(_token.loc, "no delimited expr???");
        }
        break;

      case Tk::DollarLeftBrace:
        {
          auto open = ast::Terminal(_token.loc, ast::Trivia{});
          fwd();
          ast::SmallNodeVec<1> text;
          if (_token.id == Tk::Text) {
            text.emplace_back(std::make_unique<ast::Literal>(
              ast::Literal::Kind::String, _token.loc, ast::Trivia{}
            ));
            fwd();
          }
          auto close = ast::Terminal(_token.loc, ast::Trivia{});
          if (_token.id == Tk::RightBrace) {
            fwd();
          } else {
            // ERROR
            error(_token.loc, "missing RightBrace");
          }
          pieces.emplace_back(std::make_unique<ast::Delimited>(
            std::move(open), std::move(close), std::move(text)
          ));
        }
        break;

      case Tk::Text:
        pieces.emplace_back(std::make_unique<ast::Literal>(
          ast::Literal::Kind::String, _token.loc, ast::Trivia{}
        ));
    }
  }
}

unsigned Parser::prefixPrecedence(unsigned flags) const {
  switch (_token.id) {
    default:
      return 0;

    case Tk::Comma:
      if (flags & EF_NoComma) {
        return 0;
      }
      return 2;

    case Tk::DotDot:
      return 3;

    case Tk::Minus:
    case Tk::Plus:
    case Tk::Not:
    case Tk::Tilde:
      return 13;

    case Tk::Star:
    case Tk::StarStar:
    case Tk::And:
      return 14;

    case Tk::Dot:
      return 15;

    case Tk::ColonColon:
      return 16;
  }
}

std::pair<unsigned, unsigned>
Parser::infixPostfixPrecedence(unsigned flags) const {
  switch (_token.id) {
    default:
      return {0, 0};

    case Tk::PercentEqual:
    case Tk::CaretEqual:
    case Tk::StarEqual:
    case Tk::StarStarEqual:
    case Tk::AndEqual:
    case Tk::MinusEqual:
    case Tk::PlusEqual:
    case Tk::BarEqual:
    case Tk::SlashEqual:
    case Tk::LessLessEqual:
    case Tk::GreaterGreaterEqual:
    case Tk::Equal:
      return {1 | EF_RTL, 0};

    case Tk::Comma:
      if (flags & EF_NoComma) {
        return {0, 0};
      }
      return {2, 2};

    case Tk::DotDot:
      return {3, 3};

    case Tk::BarBar:
      return {4, 0};

    case Tk::AndAnd:
      return {5, 0};

    case Tk::And:
      return {6, 2};

    case Tk::Caret:
    case Tk::Bar:
      return {6, 0};

    case Tk::EqualEqual:
    case Tk::NotEqual:
      return {7, 0};

    case Tk::Less:
    case Tk::LessEqual:
    case Tk::Greater:
    case Tk::GreaterEqual:
      return {8, 0};

    case Tk::LessLess:
    case Tk::GreaterGreater:
      return {9, 0};

    case Tk::Minus:
    case Tk::Plus:
      return {10, 0};

    case Tk::Percent:
    case Tk::Slash:
    case Tk::Star:
      return {11, 0};

    case Tk::StarStar:
      return {12 | EF_RTL, 0};

    case Tk::Not:
    case Tk::Question:
      return {0, 14};

    case Tk::Dot:
      return {15, 0};

    case Tk::ColonColon:
      return {16, 0};
  }
}

void Parser::collectTrivia() {
  // This function is called by fwd(), so that function must be avoided here.
  source::SpanRef span{_token.loc, {}};

  while (true) {
    switch (_token.id) {
      default:
        break;

      case Tk::CommentLine:
        do {
          span.second = _token.loc;
          _token = _lexer();
        } while (_token.id != Tk::NewLine && _token.id != Tk::EndOfInput);
        break;

      case Tk::Whitespace:
        span.second = _token.loc;
        _token = _lexer();
        continue;

      case Tk::CommentBlockStart:
        while (true) {
          _token = _lexer();
          if (_token.id == Tk::CommentBlockEnd) {
            span.second = _token.loc;
            _token = _lexer();
            break;
          } else if (_token.id == Tk::EndOfInput) {
            span.second = _token.loc;
            // ERROR
            error(_token.loc, "missing CommentBlockEnd");
            break;
          } 
        }
        continue;
    }

    break;
  }

  if (span.second.isValid()) {
    _trivia = ast::Trivia{span};
  } else {
    _trivia = ast::Trivia{};
  }
}

void Parser::fwd() {
  _token = _lexer();
  collectTrivia();
  if (_token.id == Tk::Ident) {
    auto fileText = _session->locator().fileText(_session->file());
    auto start = _session->locator().find(_token.loc).index;
    auto end = _session->locator().findNext(_token.loc).index;
    _kw = _keywords[fileText.substr(start, end - start)];
  }
}

template<typename TMsg, typename... Args>
void Parser::error(source::LocId locId, TMsg &&msg, Args &&...args) {
  auto loc = _session->locator().find(locId);
  auto file = _session->locator().fileName(loc.file);
  fmt::print(stderr, "error: {}:{}:{}: ", file, loc.line, loc.column);
  fmt::print(stderr, std::forward<TMsg>(msg), std::forward<Args>(args)...);
  fmt::print(stderr, "\n");
}
