#include "lava/syn/parser.h"
#include "lava/syn/lexer.h"
#include <cstdio>
#include <charconv>
#include <cassert>

using namespace lava::syn;

static const unsigned CallPrec = 17;

#define ERROR(err) \
  fprintf(stderr, "%s:%d:%d: error: %s\n", \
          token.doc->name.c_str(), \
          token.start.line, token.start.column, err)

Parser::Parser(Lexer &lexer) noexcept
  : lexer{&lexer}
{
  next();
}

void Parser::next() {
  do {
    token = lexer->lex();
  } while (token.what == TkWhitespace
        || token.what == TkLineComment
        || token.what == TkBlockComment);
}

Token Parser::take() {
  Token t = token;
  next();
  return t;
}

std::unique_ptr<Item> Parser::parse_item() {
  switch (token.what) {
  case TkFun:
    return parse_fun_item();

  case TkIdent:
    return parse_var_item();

  default:
    return nullptr;
  }
}

std::unique_ptr<VarDeclItem> Parser::parse_var_item() {
  assert(token.what == TkIdent);
  auto type = parse_expr(PF_NoComma);

  VarDeclsWithDelimiter decls;
  while (true) {
    auto decl = parse_var_decl();
    if (decl.has_value()) {
      if (token.what == TkComma) {
        decls.emplace_back(VarDeclWithDelimiter{*std::move(decl), take()});
      } else {
        decls.emplace_back(VarDeclWithDelimiter{*std::move(decl)});
        break;
      }
    } else {
      break;
    }
  }
  if (decls.empty()) {
    ERROR("missing variable name");
  }

  Token semi = token;
  if (token.what != TkSemi) {
    ERROR("missing ';' after var decl");
  } else {
    next();
  }

  return std::make_unique<VarDeclItem>(
    std::move(type), std::move(decls), semi);
}

std::optional<VarDecl> Parser::parse_var_decl() {
  if (token.what != TkIdent) {
    return std::nullopt;
  }
  Token name = take();

  if (token.what == TkEq) {
    auto init = parse_var_init();
    if (!init) {
      ERROR("expected initializer expression");
      return std::nullopt;
    }
    return VarDecl{name, *std::move(init)};
  } else {
    return VarDecl{name};
  }
}

std::optional<VarInit> Parser::parse_var_init() {
  assert(token.what == TkEq);
  Token eq = take();

  auto expr = parse_expr(PF_NoComma);
  if (!expr) {
    ERROR("missing expr");
    return std::nullopt;
  }
  return VarInit{eq, std::move(expr)};
}

std::unique_ptr<FunItemBase> Parser::parse_fun_item() {
  assert(token.what == TkFun);
  Token fun = take();

  if (token.what != TkIdent) {
    ERROR("missing fun name");
    return nullptr;
  }
  Token name = take();

  auto args = parse_arg_list();
  if (!args) {
    ERROR("missing args");
    return nullptr;
  }

  if (token.what == TkSemi) {
    return std::make_unique<FunDeclItem>(fun, name, *std::move(args), take());
  } else if (token.what == TkLeftBrace) {
    auto body = parse_scope_expr();
    if (!body) {
      ERROR("missing fun body");
      return nullptr;
    }
  }
  return nullptr;
}

std::optional<ArgList> Parser::parse_arg_list() {
  if (token.what != TkLeftParen) {
    return std::nullopt;
  }
  Token lparen = take();

  ArgDeclsWithDelimiter args;
  while (true) {
    auto arg = parse_arg_decl();
    if (!arg) {
      break;
    }
    if (token.what == TkComma) {
      args.emplace_back(ArgDeclWithDelimiter{*std::move(arg), take()});
    } else {
      args.emplace_back(ArgDeclWithDelimiter{*std::move(arg)});
      break;
    }
  }

  if (token.what != TkRightParen) {
    ERROR("missing ')'");
    return std::nullopt;
  }
  return ArgList{lparen, take(), std::move(args)};
}

std::optional<ArgDecl> Parser::parse_arg_decl() {
  if (token.what != TkIdent) {
    return std::nullopt;
  }
  auto type = parse_expr(PF_NoComma);

  if (token.what != TkIdent) {
    ERROR("missing var name");
    return std::nullopt;
  }
  Token name = take();

  if (token.what == TkEq) {
    auto init = parse_var_init();
    return ArgDecl{std::move(type), name, *std::move(init)};
  } else {
    return ArgDecl{std::move(type), name};
  }
}

std::unique_ptr<Expr> Parser::parse_expr(int flags, unsigned prec) {
  std::unique_ptr<Expr> expr;

  if (auto prefix_prec = get_prefix_prec(token.what, flags); prefix_prec > 0) {
    Token op = take();
    auto right = parse_expr(flags, prefix_prec);
    if (!right) {
      ERROR("no expression after prefix op");
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
      Token op = take();
      unsigned ltr_offset = is_rtl_operator(op.what) ? 0 : 1;
      auto right = parse_expr(flags, infix_prec + ltr_offset);
      if (right) {
        expr = std::make_unique<BinaryExpr>(
          op, std::move(expr), std::move(right));
      } else if (get_postfix_prec(token.what, flags) >= prec) {
        expr = std::make_unique<PostfixExpr>(op, std::move(expr));
      } else {
        break;
      }
    } else if (get_postfix_prec(token.what, flags) >= prec) {
      Token op = take();
      expr = std::make_unique<PostfixExpr>(op, std::move(expr));
    } else if ((token.what == TkLeftParen
             || token.what == TkLeftSquareBracket)
            && CallPrec >= prec) {
      expr = parse_invoke_expr(std::move(expr));
    } else {
      break;
    }
  }

  return expr;
}

std::optional<ScopeExpr> Parser::parse_scope_expr() {
  assert(token.what == TkLeftBrace);
  Token lbrace = take();

  ExprsWithDelimiter exprs;
  while (true) {
    auto expr = parse_expr();
    if (!expr) {
      break;
    }
    if (token.what != TkSemi) {
      ERROR("missing ';'");
      return std::nullopt;
    }
    exprs.emplace_back(ExprWithDelimiter{std::move(expr), take()});
  }

  if (token.what != TkRightBrace) {
    ERROR("missing '}'");
    return std::nullopt;
  }
  return ScopeExpr{lbrace, take(), std::move(exprs)};
}

std::unique_ptr<InvokeExpr>
Parser::parse_invoke_expr(std::unique_ptr<Expr> left) {
  Token lbracket = take();

  ExprsWithDelimiter args;
  while (true) {
    auto expr = parse_expr(PF_NoComma);
    if (!expr) {
      break;
    }
    if (token.what == TkComma) {
      args.emplace_back(ExprWithDelimiter{std::move(expr), take()});
    } else {
      args.emplace_back(ExprWithDelimiter{std::move(expr)});
      break;
    }
  }

  Token rbracket = token;
  if (lbracket.what == TkLeftParen && rbracket.what != TkRightParen) {
    ERROR("missing ')'");
  } else if (lbracket.what == TkLeftSquareBracket
          && rbracket.what != TkRightSquareBracket) {
    ERROR("missing ']'");
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
      Token lparen = take();
      auto inner = parse_expr();
      if (token.what != TkRightParen) {
        ERROR("missing right paren");
        return expr;
      }
      Token rparen = take();
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
  case TkEq:
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

bool Parser::is_rtl_operator(int op) {
  switch (op) {
  default:
    return false;

  case TkPercentEq:
  case TkHatEq:
  case TkAndEq:
  case TkStarEq:
  case TkStarStar:
  case TkStarStarEq:
  case TkMinusEq:
  case TkPlusEq:
  case TkEq:
  case TkOrEq:
  case TkLessLessEq:
  case TkLessMinusLessEq:
  case TkGreaterGreaterEq:
  case TkGreaterMinusGreaterEq:
  case TkSlashEq:
    return true;
  }
}
