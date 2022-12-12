#include "lava/lava.h"
#include "lava/syn/parser-base.h"
#include <cassert>

using namespace lava::syn;

bool ParserBase::parse_comment_line(bool skip_newline) {
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
        return true;
      } else {
        _tokens.emplace_back(token, _trivia);
        _trivia.clear();
        return false;
      }

    case Tk::EndOfInput:
      _tokens.emplace_back(token, _trivia);
      _trivia.clear();
      return false;

    default:
      LAVA_UNREACHABLE();
    }
  }
}

bool ParserBase::parse_comment_block() {
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

Token ParserBase::take() {
  if (_current_token == _tokens.size()) {
    _current_token = 0;
    _tokens.clear();
    peek();
  }

  return std::move(_tokens[_current_token++]);
}

Token &ParserBase::peek(unsigned lookahead, bool skip_newline) {
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

      _tokens.emplace_back(token, _trivia);
      _trivia.clear();
    } while (_current_token + lookahead >= _tokens.size());
  }

  return _tokens[_current_token + lookahead];
}
