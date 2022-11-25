#include "lava/lava.h"
#include "lava/syn/parser.h"
#include <cassert>

using namespace lava::syn;

Parser::Parser(src::SourceFile &src)
  : _lexer{src}
  , _trivia{}
  , _tokens{}
  , _current_token{0}
{}

NodePtr Parser::operator()(unsigned flags) {
  auto list = std::make_unique<Adjacent>();
  while (peek().id() != Tk::EndOfInput) {
    list->chain.emplace_back(parse_expr(list.get(), 1));
  }
  return list;
}

// Operator precedence {{{

unsigned Parser::prec_prefix(Tk tk, unsigned flags) {
  switch (tk) {
  case Tk::Comma:
    if (flags & EF_NoComma) {
      return 0;
    }
    return 3;

  case Tk::DotDot:
    return 4;

  case Tk::Tilde:
  case Tk::Excl:
  case Tk::Minus:
  case Tk::Plus:
  case Tk::Star:
  case Tk::StarStar:
  case Tk::And:
  case Tk::MinusMinus:
  case Tk::PlusPlus:
    return 16;

  case Tk::Dot:
    return 17;

  case Tk::ColonColon:
    return 18;

  default:
    return 0;
  }
}

std::pair<unsigned, unsigned>
Parser::prec_infix_postfix(Tk tk, unsigned flags) {
  switch (tk) {
  case Tk::Semicolon:
    return std::make_pair(0, 1);

  case Tk::NewLine:
    if (flags & EF_NewLine) {
      return std::make_pair(0, 1);
    }
    return std::make_pair(0, 0);

  case Tk::Equal:
  case Tk::PercentEqual:
  case Tk::HatEqual:
  case Tk::AndEqual:
  case Tk::StarEqual:
  case Tk::StarStarEqual:
  case Tk::MinusEqual:
  case Tk::PlusEqual:
  case Tk::BarEqual:
  case Tk::LessLessEqual:
  case Tk::GreaterGreaterEqual:
  case Tk::SlashEqual:
    return std::make_pair(2 | EF_RTL, 0);

  case Tk::Comma:
    if (flags & EF_NoComma) {
      return std::make_pair(0, 0);
    }
    return std::make_pair(3, 3);

  case Tk::DotDot:
    return std::make_pair(4, 4);

  case Tk::BarBar:
    return std::make_pair(5, 0);

  case Tk::AndAnd:
    return std::make_pair(6, 0);

  case Tk::ExclEqual:
  case Tk::EqualEqual:
    return std::make_pair(7, 0);

  case Tk::Bar:
    return std::make_pair(8, 8);

  case Tk::Hat:
    return std::make_pair(9, 0);

  case Tk::And:
    return std::make_pair(10, 10);

  case Tk::Less:
  case Tk::Greater:
    if (flags & EF_Angle) {
      return std::make_pair(0, 0);
    }
    return std::make_pair(11, 0);

  case Tk::LessEqual:
  case Tk::GreaterEqual:
    return std::make_pair(11, 0);

  case Tk::LessLess:
  case Tk::GreaterGreater:
    return std::make_pair(12, 0);

  case Tk::Minus:
  case Tk::Plus:
    return std::make_pair(13, 0);

  case Tk::Percent:
  case Tk::Star:
  case Tk::Slash:
    return std::make_pair(14, 0);

  case Tk::StarStar:
    return std::make_pair(15 | EF_RTL, 0);

  case Tk::MinusMinus:
  case Tk::PlusPlus:
    return std::make_pair(0, 16);

  case Tk::Dot:
    return std::make_pair(17, 0);

  case Tk::ColonColon:
    return std::make_pair(18, 0);

  default:
    return std::make_pair(0, 0);
  }
}

// }}}

// Operator precedence parsing {{{

NodePtr Parser::parse_expr(Tree *parent, unsigned flags) {
  if (peek().id() == Tk::EndOfInput) {
    return nullptr;
  }

  NodePtr expr = parse_expr_prefix(parent, flags);
  return parse_expr_left(parent, std::move(expr), flags);
}

NodePtr Parser::parse_expr_terminal(Tree *parent) {
  switch (peek().id()) {
  case Tk::IntLit:
  case Tk::HexLit:
  case Tk::BinLit:
  case Tk::FloatLit:
  case Tk::Ident:
  case Tk::Variable:
    return std::make_unique<Leaf>(take());

  // case Tk::Text: ???

  // case Tk::Backtick:
  // case Tk::SingleQuote:
  // case Tk::DoubleQuote:

  default:
    return nullptr;
  }
}

NodePtr Parser::parse_expr_prefix(Tree *parent, unsigned flags) {
  if (auto expr = parse_expr_bracketed(parent, flags)) {
    return expr;
  }

  unsigned prec = prec_prefix(peek().id(), flags);
  if (prec != 0) {
    auto prefix = std::make_unique<Unary>();
    prefix->is_postfix = false;
    prefix->op = std::make_unique<Leaf>(take());
    prefix->expr = parse_expr(prefix.get(), prec);
    return prefix;
  }

  return parse_expr_terminal(parent);
}

NodePtr Parser::parse_expr_bracketed(Tree *parent, unsigned flags) {
  Tk close_tk;
  switch (peek().id()) {
  case Tk::LeftParen:
    close_tk = Tk::RightParen;
    break;

  case Tk::LeftSquareBracket:
    close_tk = Tk::RightSquareBracket;
    break;

  case Tk::LeftBrace:
    close_tk = Tk::RightBrace;
    break;

  case Tk::Less:
    if (flags & EF_Angle) {
      close_tk = Tk::Greater;
    } else {
      return nullptr;
    }
    break;

  case Tk::Backtick:
  case Tk::SingleQuote:
  case Tk::DoubleQuote:
    close_tk = peek().id();
    break;

  default:
    return nullptr;
  }

  // TODO: if (flags & EF_String)

  auto bk = std::make_unique<Bracketed>();
  bk->open = std::make_unique<Leaf>(take());
  bk->expr = parse_expr(bk.get(), (flags & EF_FlagMask) | 1);
  if (peek().id() == close_tk) {
    bk->close = std::make_unique<Leaf>(take());
  } else {
    // Expected close operator
  }
  return bk;
}

NodePtr Parser::parse_expr_left(Tree *parent, NodePtr left,
                                unsigned prec_and_flags) {
  unsigned left_prec = prec_and_flags & EF_PrecMask;
  unsigned flags = prec_and_flags & EF_FlagMask;

  // At this point, we have `left` `op` ...
  // There are three possibilities for `op`:
  // - The operator is the same: Append to the current infix chain.
  // - Precedence is lower than `left_prec`: It will be handled up the
  //   recursion chain.
  // - Precedence is equal to `left_prec`
  // - Precedence is greater than `left_prec` (and not RTL): Use the
  //   right-recursive parser.
  while (true) {
    auto tk_op = peek().id();
    auto [infix_prec, postfix_prec] = prec_infix_postfix(tk_op, flags);
    infix_prec &= EF_PrecMask;

    if ((postfix_prec | infix_prec) == 0) {
      // Nothing to see here boys... 🔭🐧
      return left;
    }

    if (infix_prec >= left_prec) {
      auto op = take();
      auto right = parse_expr_prefix(parent, flags & EF_FlagMask);
      if (right) {
        auto bin = std::make_unique<Infix>();
        right = parse_expr_right(bin.get(), std::move(right), infix_prec);
        bin->first = std::move(left);
        bin->chain.emplace_back(
          std::make_unique<Leaf>(std::move(op)),
          std::move(right)
        );
        left = std::move(bin);
        continue;
      }
    }

    if (postfix_prec >= left_prec) {
      auto postfix = std::make_unique<Unary>();
      postfix->is_postfix = true;
      postfix->op = std::make_unique<Leaf>(take());
      postfix->expr = std::move(left);
      left = std::move(postfix);
    }

    return left;
  }
}

NodePtr Parser::parse_expr_right(Tree *parent, NodePtr left,
                                 unsigned prec_and_flags) {
  unsigned left_prec = prec_and_flags & EF_PrecMask;
  unsigned right_prec = prec_infix_postfix(
    peek().id(), prec_and_flags & EF_FlagMask
  ).first;

#if defined(__has_builtin) && __has_builtin(__builtin_ctz)
  static_assert(__builtin_ctz(EF_RTL) == 8);
#endif
  unsigned rtl_bit = (right_prec & EF_RTL) >> 8;
  right_prec &= ~EF_RTL;

  while ((right_prec & EF_PrecMask) + rtl_bit > left_prec) {
    auto op = take();
    NodePtr right = parse_expr(parent, right_prec);
    if (!right) {
      return nullptr;
    }

    auto bin = std::make_unique<Infix>();
    bin->is_right_recursive = true;
    bin->first = std::move(right);
    bin->chain.emplace_back(
      std::make_unique<Leaf>(std::move(op)),
      std::move(left)
    );
    left = std::move(bin);

    right_prec = prec_infix_postfix(
      peek().id(), prec_and_flags & EF_FlagMask
    ).first;
    rtl_bit = (right_prec & EF_RTL) >> 8;
    right_prec &= ~EF_RTL;
  }

  return left;
}

// }}}

// Comments {{{

bool Parser::parse_comment_line(bool skip_newline) {
  while (true) {
    SimpleToken token = _lexer();
    switch (token.id()) {
    case Tk::CommentLineText:
    case Tk::CommentKeyword:
      _trivia.emplace_back(token);
      continue;

    case Tk::NewLine:
      if (skip_newline) {
        _trivia.emplace_back(token);
      } else {
        _tokens.emplace_back(token, _trivia);
        _trivia.clear();
      }
      return true;

    case Tk::EndOfInput:
      _tokens.emplace_back(token, _trivia);
      _trivia.clear();
      return false;

    default:
      LAVA_UNREACHABLE();
    }
  }
}

bool Parser::parse_comment_block() {
  while (true) {
    SimpleToken token = _lexer();
    switch (token.id()) {
    case Tk::CommentBlockStart:
      if (!parse_comment_block()) {
        return false;
      }
      break;

    case Tk::CommentBlockEnd:
      _trivia.emplace_back(token);
      return true;

    case Tk::CommentBlockText:
    case Tk::CommentKeyword:
      _trivia.emplace_back(token);
      break;

    case Tk::NewLine:
      _trivia.emplace_back(token);
      break;

    case Tk::EndOfInput:
      _tokens.emplace_back(token, _trivia);
      _trivia.clear();
      return false;

    default:
      LAVA_UNREACHABLE();
    }
  }
}

// }}}

// Tokens {{{

Token Parser::take() {
  if (_current_token == _tokens.size()) {
    _current_token = 0;
    _tokens.clear();
    peek();
  }

  ++_current_token;
  return std::move(_tokens[_current_token - 1]);
}

Token &Parser::peek(unsigned lookahead, bool skip_newline, bool skip_trivia) {
  if (_current_token + lookahead >= _tokens.size()) {
    if (!_tokens.empty() && _tokens.back().id() == Tk::EndOfInput) {
      return _tokens.back();
    }

    do {
      SimpleToken token = _lexer();
      if (token.id() == Tk::EndOfInput) {
        return _tokens.emplace_back(token, _trivia);
        _trivia.clear();
      }

      if (skip_trivia) {
        switch (token.id()) {
        case Tk::NewLine:
          if (skip_newline) {
            _trivia.emplace_back(token);
            continue;
          }
          break;

        case Tk::Whitespace:
          _trivia.emplace_back(token);
          continue;

        case Tk::HashBang:
        case Tk::CommentLine:
          if (!parse_comment_line(skip_newline)) {
            return _tokens.back();
          }
          continue;

        case Tk::CommentBlockStart:
          if (!parse_comment_block()) {
            fmt::print(stderr, "Unterminated comment block.");
            return _tokens.back();
          }
          continue;

        case Tk::CommentLineText:
        case Tk::CommentBlockText:
        case Tk::CommentBlockEnd:
        case Tk::CommentKeyword:
          assert(!"parse_comment_* should take this token");
          break;

        default:
          break;
        }
      }

      _tokens.emplace_back(token, _trivia);
      _trivia.clear();
    } while (_current_token + lookahead >= _tokens.size());
  }

  return _tokens[_current_token + lookahead];
}

// }}}
