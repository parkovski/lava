#include "lava/syn/parser-ir.h"

using namespace lava::syn;

void IRParser::operator()() {
  while (try_parse_top()) {
  }
  if (peek().id() != Tk::EndOfInput) {
    error("extraneous junk at end of file");
  }
}

bool IRParser::try_parse_top() {
  auto const &tk = peek();
  if (tk.id() == Tk::EndOfInput) {
    return false;
  }

  switch (tk.keyword()) {
  case Kw::Fun:
    parse_fun();
    break;

  case Kw::Const:
  case Kw::Mutable:
    parse_var();
    expect(Tk::Semicolon, "expected ';'");
    do {
      take();
    } while (peek().id() == Tk::Semicolon);
    break;

  default:
    return false;
  }

  return true;
}

void IRParser::parse_scoped_name() {
  expect(Tk::Ident, "expected identifier");
  take();
  while (peek().id() == Tk::Dot) {
    take();
    expect(Tk::Ident, "expected identifier");
  }
}

void IRParser::parse_var() {
  auto mutability = take(); // const or mutable
  //bool is_mutable = decl_word.keyword() == Kw::Mutable;
  expect(Tk::Ident, "expected type name");
  parse_scoped_name();
  expect(Tk::Ident, "expected variable name");
  take();
  while (peek().id() == Tk::Comma) {
    take();
    expect(Tk::Ident, "expected variable name");
    take();
  }
}

void IRParser::parse_fun() {
  auto fun_kw = take();

  expect(Tk::Ident, "expected function name");
  take();

  expect(Tk::LeftParen, "expected arguments");
  auto lparen = take();

  parse_fun_args();

  expect(Tk::RightParen, "expected ')' after arguments");
  auto rparen = take();

  expect(Tk::LeftBrace, "expected function body");
  auto lbrace = take();

  while (try_parse_instr()) {
    expect(Tk::Semicolon, "expected ';'");
    do {
      take();
    } while (peek().id() == Tk::Semicolon);
  }

  expect(Tk::RightBrace, "expected '}'");
  take();
}

void IRParser::parse_fun_args() {
  do {
    auto kw = peek().keyword();
    if (kw == Kw::Const || kw == Kw::Mutable) {
      take();
    }

    expect(Tk::Ident, "expected type name");
    parse_scoped_name();
    expect(Tk::Ident, "expected argument name");
    take();
  } while (peek().id() == Tk::Comma && (take(), true));
}

bool IRParser::try_parse_instr() {
  if (peek().id() != Tk::Ident) {
    return false;
  }

  auto op_word = take();

  if (peek().id() == Tk::Colon) {
    // Label
    take();
    return true;
  }

  parse_literal(); // arg0

  if (peek().id() == Tk::Comma) {
    take();
    parse_literal(); // arg1

    if (peek().id() == Tk::Comma) {
      take();
      parse_literal(); // arg2
    }
  }

  return true;
}

void IRParser::parse_literal() {
  switch (peek().id()) {
  case Tk::Ident:
  case Tk::IntLit:
  case Tk::HexLit:
  case Tk::FloatLit:
  case Tk::Variable:
    take();
    break;

  default:
    error("expected variable or literal");
  }
}
