#include "lava/syn/parser.h"
#include <cstdio>
#include <charconv>

using namespace lava::syn;

Parser::Parser(Lexer &lexer) noexcept
  : lexer{&lexer}
{
  next();
}

std::unique_ptr<Expr> Parser::parse_expr(unsigned prec, int flags) {
  std::unique_ptr<Expr> expr;

  if (auto prefix_prec = get_prefix_prec(token.what, flags); prefix_prec > 0) {
    Token op = token;
    next();
    auto right = parse_expr(prefix_prec, flags);
    if (!right) {
      fprintf(stderr, "no expression after prefix op\n");
      return nullptr;
    }
    expr = std::make_unique<PrefixExpr>(op, std::move(right));
  } else {
    expr = parse_primary();
    if (!expr) {
      return nullptr;
    }
  }

  while (true) {
    auto infix_prec = get_infix_prec(token.what, flags);
    if (infix_prec >= prec) {
      Token op = token;
      next();
      auto right = parse_expr(infix_prec, flags);
      if (right) {
        expr = std::make_unique<BinaryExpr>(
          op, std::move(expr), std::move(right));
      } else if (get_postfix_prec(token.what, flags) >= prec) {
        expr = std::make_unique<PostfixExpr>(op, std::move(expr));
      } else {
        break;
      }
    } else if (get_postfix_prec(token.what, flags) >= prec) {
      Token op = token;
      next();
      expr = std::make_unique<PostfixExpr>(op, std::move(expr));
    } else if (token.what == TkLeftParen
            || token.what == TkLeftSquareBracket) {
      expr = parse_invoke_expr(std::move(expr));
    } else {
      break;
    }
  }

  return expr;
}

void Parser::next() {
  do {
    token = lexer->lex();
  } while (token.what == TkWhitespace
        || token.what == TkLineComment
        || token.what == TkBlockComment);
}

std::unique_ptr<InvokeExpr>
Parser::parse_invoke_expr(std::unique_ptr<Expr> left) {
  Token lbracket = token;
  next();

  ExprWithDelimiter arg;
  std::vector<ExprWithDelimiter> args;
  while (true) {
    arg.expr = parse_expr(1, PF_NoComma);
    if (!arg.expr) {
      break;
    }
    if (token.what == TkComma) {
      arg.delimiter = token;
      next();
      args.emplace_back(std::move(arg));
    } else {
      arg.delimiter = std::nullopt;
      args.emplace_back(std::move(arg));
      break;
    }
  }

  Token rbracket = token;
  if (lbracket.what == TkLeftParen && rbracket.what != TkRightParen) {
    fprintf(stderr, "missing ')'\n");
  } else if (lbracket.what == TkLeftSquareBracket
          && rbracket.what != TkRightSquareBracket) {
    fprintf(stderr, "missing ']'\n");
  } else {
    next();
  }

  return std::make_unique<InvokeExpr>(
    std::move(left),
    lbracket,
    rbracket,
    std::move(args)
  );
}

std::unique_ptr<Expr> Parser::parse_primary() {
  std::unique_ptr<Expr> expr;

  switch (token.what) {
  default:
    expr = nullptr;
    break;

  case TkIntLiteral:
    {
      uint64_t u = 0;
      for (auto c : token.text()) {
        u *= 10;
        u += c - '0';
      }
      expr = std::make_unique<LiteralExpr>(token, u);
      next();
    }
    break;

  case TkHexLiteral:
    {
      uint64_t u = 0;
      for (auto c : token.text().substr(2)) {
        u *= 16;
        if (c >= '0' && c <= '9') {
          u += c - '0';
        } else if (c >= 'a' && c <= 'f') {
          u += c - 'a' + 10;
        } else if (c >= 'A' && c <= 'F') {
          u += c - 'A' + 10;
        }
      }
      expr = std::make_unique<LiteralExpr>(token, u);
      next();
    }
    break;

  case TkBinLiteral:
    {
      uint64_t u = 0;
      for (auto c : token.text().substr(2)) {
        u *= 2;
        u += c - '0';
      }
      expr = std::make_unique<LiteralExpr>(token, u);
      next();
    }
    break;

  case TkFloatLiteral:
    {
      auto first = token.text().data();
      auto last = first + token.text().size();
      float f = 0;
      std::from_chars(first, last, f);
      expr = std::make_unique<LiteralExpr>(token, f);
      next();
    }
    break;

  case TkDoubleLiteral:
    {
      auto first = token.text().data();
      auto last = first + token.text().size();
      double d = 0;
      std::from_chars(first, last, d);
      expr = std::make_unique<LiteralExpr>(token, d);
      next();
    }
    break;

  case TkStringLiteral:
    expr = std::make_unique<LiteralExpr>(token, token.text());
    next();
    break;

  case TkIdent:
    expr = std::make_unique<IdentExpr>(token);
    next();
    break;

  case TkLeftParen:
    {
      Token lparen = token;
      next();
      auto inner = parse_expr();
      if (token.what != TkRightParen) {
        fprintf(stderr, "missing right paren\n");
        return expr;
      }
      Token rparen = token;
      next();
      expr = std::make_unique<ParenExpr>(lparen, rparen, std::move(inner));
    }
    break;
  }

  return expr;
}

unsigned Parser::get_prefix_prec(int op, int flags) {
  switch (op) {
  default:
    return 0;

  case TkComma:
    return (flags & PF_NoComma) ? 0 : 2;

  case TkDotDot:
    return 3;

  case TkTilde:
  case TkExcl:
  case TkMinus:
  case TkPlus:
    return 14;

  case TkStar:
  case TkStarStar:
  case TkAnd:
    return 15;

  case TkMinusMinus:
  case TkPlusPlus:
    return 16;

  case TkDot:
    return 18;
  }
}

unsigned Parser::get_infix_prec(int op, int flags) {
  switch (op) {
  default:
    return 0;

  case TkPercentEq:
  case TkHatEq:
  case TkAndEq:
  case TkStarEq:
  case TkStarStarEq:
  case TkMinusEq:
  case TkPlusEq:
  case TkOrEq:
  case TkLessLessEq:
  case TkLessMinusLessEq:
  case TkGreaterGreaterEq:
  case TkGreaterMinusGreaterEq:
  case TkSlashEq:
    return 1;

  case TkComma:
    return (flags & PF_NoComma) ? 0 : 2;

  case TkDotDot:
    return 3;

  case TkOrOr:
    return 4;

  case TkAndAnd:
    return 5;

  case TkEqEq:
  case TkExclEq:
    return 6;

  case TkAnd:
    return 7;

  case TkHat:
  case TkOr:
    return 8;

  case TkLess:
  case TkLessEq:
  case TkGreater:
  case TkGreaterEq:
    return 9;

  case TkLessLess:
  case TkLessMinusLess:
  case TkGreaterGreater:
  case TkGreaterMinusGreater:
    return 10;

  case TkMinus:
  case TkPlus:
    return 11;

  case TkPercent:
  case TkStar:
  case TkSlash:
    return 12;

  case TkStarStar:
    return 13;

  case TkDot:
    return 18;
  }
}

unsigned Parser::get_postfix_prec(int op, int flags) {
  switch (op) {
  default:
    return 0;

  case TkComma:
    return (flags & PF_NoComma) ? 0 : 2;

  case TkDotDot:
    return 3;

  case TkMinusMinus:
  case TkPlusPlus:
    return 16;

  case TkExcl:
  case TkQuestion:
    return 17;
  }
}
