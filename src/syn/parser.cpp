#include "lava/syn/parser.h"
#include <cassert>

using namespace lava::syn;

#define LOG(...) fmt::print(stderr, __VA_ARGS__)

Parser::Parser(src::SourceFile &src)
  : _lexer{src}
{
  _tokens.push_back(_lexer());
}

NodePtr Parser::operator()() {
  auto statements = std::make_unique<Infix>();
  while (true) {
    // TODO parseItem();
  }
}

bool Parser::parse_item() {
  const Token &token = peek();
  switch (token.keyword()) {
  default:
    return false;

  case Kw::Fun:
    return parse_fun();

  case Kw::Type:
    return parse_type();

  case Kw::Struct:
  case Kw::Union:
    return parse_struct_or_union();
  }
  return false;
}

// Syntax:
// fun <id> '(' <args>? ')' ('->' <id>)? ';'
// fun <id>? '(' <args>? ')' ('->' <id>)? '{' body '}'
bool Parser::parse_fun() {
  auto const tk_fun = take();

  parse_scoped_id();

  if (take().id() != Tk::LeftParen) {
    LOG("expected '('");
    return false;
  }

  return false;
}

bool Parser::parse_type() {
  return false;
}

bool Parser::parse_struct_or_union() {
  return false;
}

Token Parser::take() {
  if (_current_token == _tokens.size()) {
    _current_token = 0;
    _tokens.clear();
    return _lexer();
  }

  ++_current_token;
  return std::move(_tokens[_current_token - 1]);
}

const Token &Parser::peek(unsigned lookahead) {
  if (_current_token + lookahead >= _tokens.size()) {
    if (_tokens.back().id == Tk::EndOfInput) {
      return _tokens.back();
    }

    do {
      Token token = _lexer();
      switch (token.id) {
      case Tk::EndOfInput:
        return _tokens.emplace_back(std::move(token));

      case Tk::Whitespace:
      case Tk::NewLine:
      case Tk::HashBang:
      case Tk::CommentLine:
      case Tk::CommentBlockStart:
      case Tk::CommentBlockText:
      case Tk::CommentBlockEnd:
      case Tk::CommentKeyword:
        // TODO Save as trivia.
        break;

      default:
        _tokens.emplace_back(std::move(token));
        break;
      }
    } while (_current_token + lookahead >= _tokens.size());
  }

  return _tokens[_current_token + lookahead];
}
