#include "lava/lava.h"
#include "lava/syn/parser.h"
#include <cassert>

using namespace lava::syn;

NodePtr Parser::operator()() {
  return many(&Parser::parse_top);
}

std::unique_ptr<List> Parser::many(NodePtr (Parser::*fn)()) {
  auto list = std::make_unique<List>();
  NodePtr node;
  while ((node = (this->*fn)())) {
    list->chain.emplace_back(std::move(node));
  }
  return list;
}

NodePtr Parser::parse_top() {
  auto const &tk = peek();

  if (tk.id() == Tk::EndOfInput) {
    return nullptr;
  }

  switch (tk.keyword()) {
  case Kw::Namespace:
    return std::make_unique<Namespace>(parse_namespace());

  case Kw::Interface:
    return std::make_unique<Interface>(parse_interface());

  case Kw::Struct:
  case Kw::Union:
    return parse_struct_union();

  case Kw::Enum:
    return std::make_unique<Enum>(parse_enum());

  case Kw::Type:
    return parse_type();

  case Kw::Fun:
    return parse_fun();

  //case Kw::Implement:
  //  return parse_implement();

  case Kw::Let:
  case Kw::Mutable:
  case Kw::Const:
    return parse_var();

  default:
    return parse_expr(1);
  }

  return nullptr;
}

Namespace Parser::parse_namespace() {
  assert(peek().keyword() == Kw::Namespace);
  Namespace ns;
  ns.ns_word = Leaf(take());
  ns.path = parse_scoped_name();

  expect(Tk::LeftBrace, "expected '{' after namespace");
  ns.open_brace = Leaf(take());

  ns.body = (*this)();

  expect(Tk::RightBrace, "expected '}'");
  ns.close_brace = Leaf(take());

  return ns;
}

Infix Parser::parse_scoped_name() {
  expect(Tk::Ident, "expected identifier");
  Infix infix;
  infix.first = take_as_leaf();

  while (peek().id() == Tk::Dot) {
    auto dot = take_as_leaf();
    expect(Tk::Ident, "expected identifier after '.'");
    infix.chain.emplace_back(std::move(dot), take_as_leaf());
  }

  return infix;
}

Interface Parser::parse_interface() {
  assert(peek().keyword() == Kw::Interface);
  Interface ifc;
  ifc.interface_word = Leaf(take());

  expect(Tk::Ident, "expected identifier after interface");
  ifc.name = Leaf(take());

  expect(Tk::LeftBrace, "expected '{' after interface");
  ifc.open_brace = Leaf(take());

  ifc.body = many(&Parser::parse_interface_inner);

  expect(Tk::RightBrace, "expected '}'");
  ifc.close_brace = Leaf(take());

  return ifc;
}

NodePtr Parser::parse_interface_inner() {
  auto const &tk = peek();
  switch (tk.keyword()) {
  case Kw::Type:
    return parse_type(true);
    break;

  case Kw::Fun:
    return parse_fun(true);
    break;

  case Kw::Let:
  case Kw::Mutable:
  case Kw::Const:
    return parse_var(true);
    break;

  case Kw::Requires: {
    auto name = parse_scoped_name();
    expect(Tk::Semicolon, "expected ';' after requires");
    auto stmt = std::make_unique<Unary>();
    stmt->is_postfix = true;
    stmt->op = take_as_leaf();
    stmt->expr = std::make_unique<Infix>(std::move(name));
    return stmt;
  }

  default:
    return nullptr;
  }
}

NodePtr Parser::parse_struct_union() {
  auto const &tk = peek();
  assert(tk.keyword() == Kw::Struct || tk.keyword() == Kw::Union);
  auto sunode = std::make_unique<List>();
  sunode->chain.emplace_back(take_as_leaf());

  expect(Tk::Ident, "expected identifier after struct/union");
  sunode->chain.emplace_back(take_as_leaf());

  expect(Tk::LeftBrace, "expected '{' after struct/union");
  auto body = std::make_unique<Bracketed>();
  body->open = take_as_leaf();

  body->expr = many(&Parser::parse_struct_union_inner);

  expect(Tk::RightBrace, "expected '}'");
  body->close = take_as_leaf();

  sunode->chain.emplace_back(std::move(body));
  return sunode;
}

NodePtr Parser::parse_struct_union_inner() {
  auto const &tk = peek();
  switch (tk.keyword()) {
  case Kw::Type:
    return parse_type();

  case Kw::Fun:
    return parse_fun();

  case Kw::Let:
  case Kw::Const:
  case Kw::Mutable:
    return parse_var();

  default:
    return nullptr;
  }
}

Enum Parser::parse_enum() {
  auto const &tk = peek();
  assert(tk.keyword() == Kw::Enum);
  Enum enm;
  enm.enum_word = Leaf(take());

  expect(Tk::Ident, "expected identifier after enum");
  enm.name = Leaf(take());

  expect(Tk::LeftBrace, "expected '{' after enum");
  enm.open_brace = Leaf(take());

  enm.body = parse_enum_inner();

  expect(Tk::RightBrace, "expected '}' after enum body");
  enm.close_brace = Leaf(take());

  return enm;
}

NodePtr Parser::parse_enum_inner() {
  return nullptr;
}

NodePtr Parser::parse_type(bool decl_only) {
  auto const &tk = peek();
  assert(tk.keyword() == Kw::Type);
  auto type = std::make_unique<List>();
  type->chain.emplace_back(take_as_leaf());

  expect(Tk::Ident, "expected identifier after type");
  type->chain.emplace_back(take_as_leaf());

  expect(Tk::Equal, "expected '=' after type name");
  type->chain.emplace_back(take_as_leaf());

  type->chain.emplace_back(std::make_unique<Infix>(parse_scoped_name()));

  expect(Tk::Semicolon, "expected ';'");
  type->chain.emplace_back(take_as_leaf());
  return type;
}

NodePtr Parser::parse_fun(bool decl_only) {
  auto const &tk = peek();
  assert(tk.keyword() == Kw::Fun);
  auto fun = std::make_unique<List>();

  LAVA_UNREACHABLE();
}

NodePtr Parser::parse_var(bool decl_only) {
  auto const &tk = peek();
  assert(tk.keyword() == Kw::Let
      || tk.keyword() == Kw::Const
      || tk.keyword() == Kw::Mutable);
  auto vardecl = std::make_unique<List>();
  vardecl->chain.emplace_back(take_as_leaf());

  expect(Tk::Ident, "expected variable name");
  vardecl->chain.emplace_back(take_as_leaf());

  if (peek().id() == Tk::Equal) {
    if (decl_only) {
      error("expected ';' after var name");
    }

    vardecl->chain.emplace_back(take_as_leaf());
    auto expr = parse_expr(1 | EF_NoComma);
    if (!expr) {
      error("expected initializing expression");
    }
    vardecl->chain.emplace_back(std::move(expr));
  }

  expect(Tk::Semicolon, "expected ';'");
  vardecl->chain.emplace_back(take_as_leaf());

  return vardecl;
}

// Operator precedence parsing {{{

NodePtr Parser::parse_expr(unsigned prec_and_flags) {
  if (peek().id() == Tk::EndOfInput) {
    return nullptr;
  }

  NodePtr expr = parse_expr_prefix(prec_and_flags);
  if (!expr) {
    return nullptr;
  }
  return parse_expr_left(std::move(expr), prec_and_flags);
}

NodePtr Parser::parse_expr_terminal() {
  switch (peek().id()) {
  case Tk::IntLit:
  case Tk::HexLit:
  case Tk::BinLit:
  case Tk::FloatLit:
  case Tk::Ident:
  case Tk::Variable:
    return take_as_leaf();

  // case Tk::Text: ???

  // case Tk::Backtick:
  // case Tk::SingleQuote:
  // case Tk::DoubleQuote:

  default:
    return nullptr;
  }
}

NodePtr Parser::parse_expr_bracketed(unsigned flags) {
  Tk close_tk = matching_delimiter(peek().id(), flags);
  if (close_tk == Tk::Invalid) {
    return nullptr;
  }

  // TODO: if (flags & EF_String)

  auto bk = std::make_unique<Bracketed>();
  bk->open = take_as_leaf();
  bk->expr = parse_expr((flags & EF_FlagMask) | 1);
  if (peek().id() == close_tk) {
    bk->close = std::make_unique<Leaf>(take());
  } else {
    error("expected close bracket");
  }
  return bk;
}

NodePtr Parser::parse_expr_prefix(unsigned flags) {
  if (auto expr = parse_expr_bracketed(flags)) {
    return expr;
  }

  unsigned prec = prec_prefix(peek().id(), flags);
  if (prec != 0) {
    auto prefix = std::make_unique<Unary>();
    prefix->is_postfix = false;
    prefix->op = take_as_leaf();
    prefix->expr = parse_expr(prec);
    return prefix;
  }

  return parse_expr_terminal();
}

NodePtr Parser::parse_expr_left(NodePtr left, unsigned prec_and_flags) {
  unsigned left_prec = prec_and_flags & EF_PrecMask;
  unsigned flags = prec_and_flags & EF_FlagMask;
  Tk tk_last_op = Tk::Invalid;
  unsigned infix_prec, postfix_prec;
  Infix *infix;

  while (true) {
    auto tk_op = peek().id();
    Token op;

    // First check if this is a repeated operator. This allows treating infix
    // expressions like `a + b + c` similar to Lisp's `(+ a b c)`.
    while (tk_op == tk_last_op) {
      op = take();
      auto right = parse_expr_prefix(flags | infix_prec);
      if (right) {
        right = parse_expr_right(std::move(right), flags | infix_prec);
        infix->chain.emplace_back(
          std::make_unique<Leaf>(std::move(op)),
          std::move(right)
        );
        tk_op = peek().id();
      }
    }


    // Check if there is a different operator with greater or equal precedence
    // to the last. If one exists, we want to take all operators with greater
    // precedence (see `parse_expr_right`).
    std::tie(infix_prec, postfix_prec) = prec_infix_postfix(tk_op, flags);
    unsigned rtl_bit = get_rtl_bit(infix_prec);
    bool is_delimited = postfix_prec & EF_Delimited;
    infix_prec &= EF_PrecMask;
    postfix_prec &= EF_PrecMask;

    if (infix_prec >= left_prec) {
      op = take();
      // If the operator is RTL, use the same precedence level. If it is LTR,
      // the level has to increase with recursion to avoid right grouping.
      auto right = parse_expr_prefix(flags | (infix_prec + 1 - rtl_bit));
      if (right) {
        tk_last_op = op.id();
        right = parse_expr_right(std::move(right), infix_prec);
        infix = new Infix();
        infix->first = std::move(left);
        left.reset(infix);
        infix->chain.emplace_back(
          std::make_unique<Leaf>(std::move(op)),
          std::move(right)
        );
        continue;
      }
    }

    // Not looking for infix operators anymore.
    tk_last_op = Tk::Invalid;

    if (postfix_prec >= left_prec) {
      if (postfix_prec > infix_prec) {
        // If infix_prec >= postfix_prec, the operator has already been
        // consumed. This happens in cases where an operator is both a postfix
        // and infix operator and the above attempt to treat it as infix fails.
        op = take();
      }

      if (is_delimited) {
        auto close_delimiter = matching_delimiter(op.id(), flags);
        auto bracketed = std::make_unique<Bracketed>();
        bracketed->open = std::make_unique<Leaf>(std::move(op));
        bracketed->expr = parse_expr(1);
        if (peek().id() == close_delimiter) {
          bracketed->close = std::make_unique<Leaf>(take());
        } else {
          error("expected close bracket");
        }

        auto postfix = std::make_unique<PostfixBracketed>();
        postfix->expr = std::move(left);
        postfix->bracketed = std::move(bracketed);
        left = std::move(postfix);
        continue;
      } else {
        auto postfix = std::make_unique<Unary>();
        postfix->is_postfix = true;
        postfix->op = std::make_unique<Leaf>(std::move(op));
        postfix->expr = std::move(left);
        left = std::move(postfix);
        continue;
      }
    }

    // Nothing to see here boys... 🔭🐧
    return left;
  }
}

NodePtr Parser::parse_expr_right(NodePtr left, unsigned prec_and_flags) {
  Tk tk_op = peek().id();
  unsigned flags = prec_and_flags & EF_FlagMask;
  unsigned left_prec = prec_and_flags & EF_PrecMask;
  auto [right_prec, postfix_prec] = prec_infix_postfix(tk_op, flags);

  // Add one to the precedence of RTL operators so that they will parse
  // recursively at equal precedence.
  unsigned rtl_bit = get_rtl_bit(right_prec);
  right_prec &= EF_PrecMask;

  while (right_prec + rtl_bit > left_prec) {
    auto op = take();
    NodePtr right = parse_expr(right_prec | flags);
    if (!right) {
      return nullptr;
    }

    // TODO How to make a right recursive chain (a = b = c = d)?
    auto infix = std::make_unique<Infix>();
    infix->is_right_recursive = true;
    infix->first = std::move(right);
    infix->chain.emplace_back(
      std::make_unique<Leaf>(std::move(op)),
      std::move(left)
    );
    left = std::move(infix);

    right_prec = prec_infix_postfix(peek().id(), flags).first;
    rtl_bit = (right_prec & EF_RTL) >> 8;
    right_prec &= EF_PrecMask;
  }

  return left;
}

// }}}

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
  //case Tk::Semicolon:
  //  return std::make_pair(0, 1);

  //case Tk::NewLine:
  //  if (flags & EF_NewLine) {
  //    return std::make_pair(0, 1);
  //  }
  //  return std::make_pair(0, 0);

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
      return std::make_pair(0, 16 | EF_Delimited);
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

  case Tk::LeftParen:
  case Tk::LeftSquareBracket:
  case Tk::LeftBrace:
    return std::make_pair(0, 16 | EF_Delimited);

  case Tk::Dot:
    return std::make_pair(17, 0);

  case Tk::ColonColon:
    return std::make_pair(18, 0);

  default:
    return std::make_pair(0, 0);
  }
}

constexpr Tk Parser::matching_delimiter(Tk tk, unsigned flags) {
  switch (tk) {
  case Tk::LeftParen:
    return Tk::RightParen;

  case Tk::LeftSquareBracket:
    return Tk::RightSquareBracket;

  case Tk::LeftBrace:
    return Tk::RightBrace;

  case Tk::Less:
    if (flags & EF_Angle) {
      return Tk::Greater;
    }
    return Tk::Invalid;

  case Tk::Backtick:
  case Tk::SingleQuote:
  case Tk::DoubleQuote:
    return tk;

  default:
    return Tk::Invalid;
  }
}

// }}}
