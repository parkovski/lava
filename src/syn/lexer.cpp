#include "lava/syn/lexer.h"

#include <cassert>

using namespace lava;
using namespace lava::syn;

Lexer::Lexer(src::SourceFile &source)
  : _src{&source}
  , _loc{0, 1, 1}
  , _context{}
{}

Token Lexer::operator()() {
  Tk tk;
  auto startLoc = _loc;

  if (_loc.index >= _src->size()) {
    return Token(Tk::EndOfInput, {_loc, _loc});
  } else {
    switch (context()) {
      case Context::Initial:
        tk = readInitial();
        break;

      case Context::EmbedParens:
        tk = readEmbedParens();
        break;

      case Context::EmbedBraces:
        tk = readEmbedBraces();
        break;

      case Context::StringBacktick:
        tk = readStringBacktick();
        break;

      case Context::StringDQuote:
        tk = readStringDQuote();
        break;

      case Context::StringSQuote:
        tk = readStringSQuote();
        break;

      case Context::CommentLine:
        tk = readCommentLine();
        break;

      case Context::CommentBlock:
        tk = readCommentBlock();
        break;

      default:
        assert(false && "Invalid lexer context.");
        break;
    }
  }

  if (tk == Tk::Ident) {
    return Token(kw_from_string(std::string_view{
      &(*_src)[startLoc.index], _loc.index - startLoc.index
    }), {startLoc, _loc});
  }
  return Token(tk, {startLoc, _loc});
}

Tk Lexer::readInitial() {
  char c = ch();
  char l = c | 0x20; // lowercase

  if (l >= 'a' && l <= 'z') {
    return readIdent();
  }

  if (c >= '0' && c <= '9') {
    return readNumber();
  }

  switch (c) {
    case 0:
      return Tk::EndOfInput;

    case '_':
      return readIdent();

    case ' ':
    case '\t':
      return readWhitespace();

    case '\r':
      if (ch(1) == '\n') {
        _loc.index += 2;
        ++_loc.line;
        _loc.column = 1;
        return Tk::NewLine;
      }
      return readWhitespace();

    case '\n':
      fwd();
      return Tk::NewLine;

    case '\\':
      return readEscape();

    case '$':
      return readVariable();

    case '\'':
      _context.push_back(Context::StringSQuote);
      fwd();
      return Tk::SingleQuote;

    case '"':
      _context.push_back(Context::StringDQuote);
      fwd();
      return Tk::DoubleQuote;

    case '`':
      _context.push_back(Context::StringBacktick);
      fwd();
      return Tk::Backtick;

    case '#':
      fwd();
      if (_loc.index == 1 && ch() == '!') {
        fwd();
        _context.push_back(Context::CommentLine);
        return Tk::HashBang;
      }
      return Tk::Hash;

    case '~':
      fwd();
      return Tk::Tilde;

    case '!':
      fwd();
      if (ch() == '=') {
        fwd();
        return Tk::NotEqual;
      }
      return Tk::Not;

    case '@':
      fwd();
      return Tk::At;

    case '%':
      fwd();
      if (ch() == '=') {
        fwd();
        return Tk::PercentEqual;
      }
      return Tk::Percent;

    case '^':
      fwd();
      if (ch() == '=') {
        fwd();
        return Tk::CaretEqual;
      }
      return Tk::Caret;

    case '&':
      fwd();
      c = ch();
      if (c == '&') {
        fwd();
        return Tk::AndAnd;
      } else if (c == '=') {
        fwd();
        return Tk::AndEqual;
      }
      return Tk::And;

    case '*':
      fwd();
      c = ch();
      if (c == '*') {
        fwd();
        if (ch() == '=') {
          fwd();
          return Tk::StarStarEqual;
        }
        return Tk::StarStar;
      } else if (c == '=') {
        fwd();
        return Tk::StarEqual;
      }
      return Tk::Star;

    case '-':
      fwd();
      c = ch();
      if (c == '=') {
        fwd();
        return Tk::MinusEqual;
      } else if (c == '>') {
        fwd();
        return Tk::MinusArrowRight;
      }
      return Tk::Minus;

    case '+':
      fwd();
      if (ch() == '=') {
        fwd();
        return Tk::PlusEqual;
      }
      return Tk::Plus;

    case '=':
      fwd();
      if (ch() == '=') {
        fwd();
        return Tk::EqualEqual;
      }
      return Tk::Equal;

    case '|':
      fwd();
      c = ch();
      if (c == '=') {
        fwd();
        return Tk::BarEqual;
      } else if (c == '|') {
        fwd();
        return Tk::BarBar;
      }
      return Tk::Bar;

    case '<':
      fwd();
      c = ch();
      if (c == '=') {
        fwd();
        return Tk::LessEqual;
      } else if (c == '<') {
        fwd();
        if (ch() == '=') {
          fwd();
          return Tk::LessLessEqual;
        }
        return Tk::LessLess;
      } else if (c == '#') {
        fwd();
        _context.push_back(Context::CommentBlock);
        return Tk::CommentBlockStart;
      }
      return Tk::Less;

    case '>':
      fwd();
      c = ch();
      if (c == '=') {
        fwd();
        return Tk::GreaterEqual;
      } else if (c == '>') {
        fwd();
        if (ch() == '=') {
          fwd();
          return Tk::GreaterGreaterEqual;
        }
        return Tk::GreaterGreater;
      }
      return Tk::Greater;

    case '/':
      fwd();
      c = ch();
      if (c == '=') {
        fwd();
        return Tk::SlashEqual;
      } else if (c == '*') {
        fwd();
        _context.push_back(Context::CommentBlock);
        return Tk::CommentBlockStart;
      } else if (c == '/') {
        fwd();
        _context.push_back(Context::CommentLine);
        return Tk::CommentLine;
      }
      return Tk::Slash;

    case '(':
      fwd();
      return Tk::LeftParen;

    case ')':
      fwd();
      return Tk::RightParen;

    case '[':
      fwd();
      return Tk::LeftSquareBracket;

    case ']':
      fwd();
      return Tk::RightSquareBracket;

    case '{':
      fwd();
      return Tk::LeftBrace;

    case '}':
      fwd();
      return Tk::RightBrace;

    case ';':
      fwd();
      return Tk::Semicolon;

    case ':':
      fwd();
      if (ch() == ':') {
        fwd();
        return Tk::ColonColon;
      }
      return Tk::Colon;

    case ',':
      fwd();
      return Tk::Comma;

    case '.':
      c = ch(1);
      if (c >= '0' && c <= '9') {
        return readNumber();
      }
      fwd();
      if (c == '.') {
        fwd();
        if (ch() == '.') {
          fwd();
          return Tk::DotDotDot;
        }
        return Tk::DotDot;
      }
      return Tk::Dot;

    case '?':
      fwd();
      return Tk::Question;

    default:
      return Tk::Invalid;
  }
}

Tk Lexer::readEmbedParens() {
  if (ch() == ')') {
    _context.pop_back();
    fwd();
    return Tk::LeftParen;
  }

  return readInitial();
}

Tk Lexer::readEmbedBraces() {
  if (ch() == '}') {
    _context.pop_back();
    fwd();
    return Tk::LeftBrace;
  }

  return readInitial();
}

Tk Lexer::readStringBacktick() {
  char c = ch();
  if (c == 0) {
    return Tk::EndOfInput;
  }
  if (c == '`') {
    _context.pop_back();
    fwd();
    return Tk::Backtick;
  }
  if (c == '\\') {
    return readEscape();
  }
  if (c == '$') {
    return readVariable();
  }

  while (true) {
    fwd();
    c = ch();
    if (c == 0 || c == '`' || c == '\\' || c == '$') {
      break;
    }
  }
  return Tk::Text;
}

Tk Lexer::readStringDQuote() {
  char c = ch();
  if (c == 0) {
    return Tk::EndOfInput;
  }
  if (c == '"') {
    _context.pop_back();
    fwd();
    return Tk::DoubleQuote;
  }
  if (c == '\\') {
    return readEscape();
  }
  if (c == '$') {
    return readVariable();
  }

  while (true) {
    fwd();
    c = ch();
    if (c == 0 || c == '"' || c == '\\' || c == '$') {
      break;
    }
  }
  return Tk::Text;
}

Tk Lexer::readStringSQuote() {
  char c = ch();
  if (c == 0) {
    return Tk::EndOfInput;
  }
  if (c == '\'') {
    _context.pop_back();
    fwd();
    return Tk::SingleQuote;
  }

  while (true) {
    fwd();
    c = ch();
    if (c == 0 || c == '\'') {
      break;
    }
  }
  return Tk::Text;
}

Tk Lexer::readCommentLine() {
  auto c = ch();
  if (c == '\n' || (c == '\r' && ch(1) == '\n')) {
    _context.pop_back();
    fwd();
    return Tk::NewLine;
  } else if (c == 0) {
    _context.pop_back();
    return Tk::EndOfInput;
  }

  while (true) {
    fwd();
    c = ch();
    if (c == 0 || c == '\n') {
      break;
    }
    if (c == '\r' && ch(1) == '\n') {
      break;
    }
  }
  return Tk::Text;
}

Tk Lexer::readCommentBlock() {
  char c = ch(), c1;
  if (c == 0) {
    return Tk::EndOfInput;
  }

  c1 = ch(1);
  if (c == '*' && c1 == '/') {
    _context.pop_back();
    fwd(2);
    return Tk::CommentBlockEnd;
  }
  if (c == '/' && c1 == '*') {
    // Nested comment.
    _context.push_back(Context::CommentBlock);
    fwd(2);
    return Tk::CommentBlockStart;
  }

  while (true) {
    fwd();
    c = ch();
    if (c == 0) {
      break;
    }

    c1 = ch(1);
    if ((c == '*' && c1 == '/') || (c == '/' && c1 == '*')) {
      break;
    }
  }
  return Tk::Text;
}

Tk Lexer::readWhitespace() {
  fwd();
  while (true) {
    char c = ch();
    if (c == ' ' || c == '\t' || (c == '\r' && ch(1) != '\n')) {
      fwd();
    } else {
      break;
    }
  }
  return Tk::Whitespace;
}

Tk Lexer::readIdent() {
  fwd();
  while (true) {
    char c = ch();
    char l = c | 0x20; // lowercase
    if (c == '_' || (c >= '0' && c <= '9') || (l >= 'a' && l <= 'z')) {
      fwd();
    } else {
      break;
    }
  }
  return Tk::Ident;
}

Tk Lexer::readNumber() {
  char c = ch();

  if (c == '0') {
    c = ch(1);
    if ((c | 0x20) == 'x') {
      fwd(2);
      while (true) {
        c = ch();
        char l = c | 0x20;
        if ((c >= '0' && c <= '9') || (l >= 'a' && l <= 'z') || c == '_') {
          fwd();
        } else {
          break;
        }
      }
      return Tk::HexLit;
    } else if ((c | 0x20) == 'b') {
      fwd(2);
      while (true) {
        c = ch();
        if (c == '0' || c == '1' || c == '_') {
          fwd();
        } else {
          break;
        }
      }
      return Tk::BinLit;
    }
  }

  while (true) {
    if (c >= '0' && c <= '9') {
      fwd();
      c = ch();
    } else {
      break;
    }
  }

  if (c == '.') {
    fwd();
    while (true) {
      c = ch();
      if (c >= '0' && c <= '9') {
        fwd();
      } else {
        break;
      }
    }
    if ((c | 0x20) == 'e') {
      c = ch(1);
      if (c >= '0' && c <= '9') {
        fwd(2);
      } else if (c == '+' || c == '-') {
        c = ch(2);
        if (c >= '0' && c <= '9') {
          fwd(3);
        } else {
          return Tk::FloatLit;
        }
      } else {
        return Tk::FloatLit;
      }
      while (true) {
        c = ch();
        if (c >= '0' && c <= '9') {
          fwd();
        } else {
          break;
        }
      }
    }
    return Tk::FloatLit;
  }

  return Tk::IntLit;
}

Tk Lexer::readEscape() {
  fwd();
  switch (ch()) {
    case '\\':
    case '"':
    case '\'':
    case '`':
    case 'a':
    case 'b':
    case 'e':
    case 'f':
    case 'n':
    case 'r':
    case 't':
    case 'v':
      fwd();
      return Tk::Escape;

    // case 'x':

    default:
      return Tk::Invalid;
  }
}

Tk Lexer::readVariable() {
  fwd();
  char c = ch();

  switch (c) {
    case '(':
      fwd();
      _context.push_back(Context::EmbedParens);
      return Tk::DollarLeftParen;

    case '{':
      fwd();
      _context.push_back(Context::EmbedBraces);
      return Tk::DollarLeftBrace;

    case '!':
    case '@':
    case '#':
    case '$':
    case '*':
    case '-':
    case '?':
    case '0':
    case '1':
    case '2':
    case '3':
    case '4':
    case '5':
    case '6':
    case '7':
    case '8':
    case '9':
      fwd();
      return Tk::Variable;

    default:
      break;
  }

  char l = c | 0x20; // lowercase
  if (c == '_' || (l >= 'a' && l <= 'z')) {
    fwd();
    while (true) {
      c = ch();
      l = c | 0x20;
      if (c == '_' || (c >= '0' && c <= '9') || (l >= 'a' && l <= 'z')) {
        fwd();
      } else {
        break;
      }
    }
    return Tk::Variable;
  }

  return Tk::Invalid;
}

char Lexer::ch(size_t offset) const {
  if (_loc.index + offset >= _src->size()) {
    return 0;
  }
  return (*_src)[_loc.index + offset];
}

void Lexer::fwd(size_t count) {
  if (_loc.index >= _src->size()) {
    return;
  }
  if ((*_src)[_loc.index] == '\n') {
    ++_loc.line;
    _loc.column = 1;
  } else if ((*_src)[_loc.index] == '\r' && ch(1) == '\n') {
    ++_loc.line;
    _loc.column = 1;
    ++_loc.index;
  } else {
    ++_loc.column;
  }
  ++_loc.index;
  if (count > 1) {
    fwd(count - 1);
  }
}

Lexer::Context Lexer::context() const {
  if (_context.empty()) {
    return Context::Initial;
  }
  return _context.back();
}
