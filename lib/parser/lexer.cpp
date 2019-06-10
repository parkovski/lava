#include "ash/parser/lexer.h"

#include <ostream>

using namespace ash;

std::ostream &ash::operator<<(std::ostream &os, TokenKind token) {
  const char *text;
  switch (token) {
    default:
      text = "unknown";
      break;

    case TokenKind::Invalid:
      text = "invalid";
      break;

    case TokenKind::Tilde:
      text = "tilde";
      break;

    case TokenKind::At:
      text = "at";
      break;

    case TokenKind::Dollar:
      text = "dollar";
      break;

    case TokenKind::LParen:
      text = "left parenthesis";
      break;

    case TokenKind::RParen:
      text = "right parenthesis";
      break;

    case TokenKind::LBracket:
      text = "left square bracket";
      break;

    case TokenKind::RBracket:
      text = "right square bracket";
      break;

    case TokenKind::LBrace:
      text = "left brace";
      break;

    case TokenKind::RBrace:
      text = "right brace";
      break;

    case TokenKind::Backslash:
      text = "backslash";
      break;

    case TokenKind::Semi:
      text = "semi";
      break;

    case TokenKind::Comma:
      text = "comma";
      break;

    case TokenKind::Excl:
      text = "exclamation";
      break;

    case TokenKind::ExclEq:
      text = "exclamation equal";
      break;

    case TokenKind::Mod:
      text = "modulo";
      break;

    case TokenKind::ModEq:
      text = "modulo equal";
      break;

    case TokenKind::Xor:
      text = "xor";
      break;

    case TokenKind::XorEq:
      text = "xor equal";
      break;

    case TokenKind::Eq:
      text = "equal";
      break;

    case TokenKind::EqEq:
      text = "double equal";
      break;

    case TokenKind::RArrowDbl:
      text = "right double arrow";
      break;

    case TokenKind::Minus:
      text = "minus";
      break;

    case TokenKind::MinusMinus:
      text = "double minus";
      break;

    case TokenKind::MinusEq:
      text = "minus equal";
      break;

    case TokenKind::RArrowSng:
      text = "right single arrow";
      break;

    case TokenKind::And:
      text = "and";
      break;

    case TokenKind::AndAnd:
      text = "double and";
      break;

    case TokenKind::AndEq:
      text = "and equal";
      break;

    case TokenKind::Plus:
      text = "plus";
      break;

    case TokenKind::PlusPlus:
      text = "double plus";
      break;

    case TokenKind::PlusEq:
      text = "plus equal";
      break;

    case TokenKind::Star:
      text = "star";
      break;

    case TokenKind::StarStar:
      text = "double star";
      break;

    case TokenKind::StarEq:
      text = "star equal";
      break;

    case TokenKind::Gt:
      text = "greater";
      break;

    case TokenKind::Shr:
      text = "shift right";
      break;

    case TokenKind::GtEq:
      text = "greater equal";
      break;

    case TokenKind::ShrEq:
      text = "shift right equal";
      break;

    case TokenKind::Colon:
      text = "colon";
      break;

    case TokenKind::ColonColon:
      text = "double colon";
      break;

    case TokenKind::Dot:
      text = "dot";
      break;

    case TokenKind::DotDot:
      text = "double dot";
      break;

    case TokenKind::DotDotDot:
      text = "triple dot";
      break;

    case TokenKind::Or:
      text = "or";
      break;

    case TokenKind::OrOr:
      text = "double or";
      break;

    case TokenKind::OrEq:
      text = "or equal";
      break;

    case TokenKind::RTriangle:
      text = "right triangle";
      break;

    case TokenKind::Lt:
      text = "less";
      break;

    case TokenKind::Shl:
      text = "shift left";
      break;

    case TokenKind::LtEq:
      text = "less equal";
      break;

    case TokenKind::ShlEq:
      text = "shift left equal";
      break;

    case TokenKind::Spaceship:
      text = "spaceship";
      break;

    case TokenKind::LTriangle:
      text = "left triangle";
      break;

    case TokenKind::LArrowSng:
      text = "left single arrow";
      break;

    case TokenKind::Slash:
      text = "slash";
      break;

    case TokenKind::SlashEq:
      text = "slash equal";
      break;

    case TokenKind::Eof:
      text = "EOF";
      break;

    case TokenKind::Whitespace:
      text = "whitespace";
      break;

    case TokenKind::NewLine:
      text = "new line";
      break;

    case TokenKind::LineComment:
      text = "line comment";
      break;

    case TokenKind::BlockComment:
      text = "block comment";
      break;

    case TokenKind::Id:
      text = "id";
      break;

    case TokenKind::Integer:
      text = "integer";
      break;

    case TokenKind::Float:
      text = "float";
      break;

    case TokenKind::String:
      text = "string";
      break;
  }

  return os << text;
}

int Lexer::current() const {
  if (_cur.pos >= _text.length()) {
    return -1;
  }
  return _text[_cur.pos];
}

int Lexer::next() {
  ++_cur.pos;
  ++_cur.column;
  if (_cur.pos >= _text.length()) {
    return -1;
  }
  return _text[_cur.pos];
}

void Lexer::newLine() {
  ++_cur.line;
  _cur.column = 1;
}

Token Lexer::lex() {
  if (_cur.pos >= _text.length()) {
    return {TokenKind::Eof, _cur.pos};
  }
  _prev = _cur;

  auto c = current();
  TokenKind tk;

  if (c == ' ' || c == '\t') {
    tk = lexWhitespace();
  } else if (c == '\r' || c == '\n') {
    tk = lexNewLine();
  } else if ((c >= 'a' && c <= 'z')
             || (c >= 'A' && c <= 'Z')
             || c == '_')
  {
    tk = lexId();
  } else if (c >= '0' && c <= '9') {
    tk = lexNumber();
  } else if (c == '\'' || c == '"' || c == '`') {
    tk = lexString();
  } else {
    tk = lexOperator();
  }

  return {tk, _prev.pos};
}

TokenKind Lexer::lexWhitespace() {
  int c = current();
  while (c == ' ' || c == '\t') {
    c = next();
  }
  return TokenKind::Whitespace;
}

TokenKind Lexer::lexNewLine() {
  if (current() == '\r') {
    if (next() == '\n') {
      next();
    }
    newLine();
  } else if (current() == '\n') {
    next();
    newLine();
  }
  return TokenKind::NewLine;
}

TokenKind Lexer::lexId() {
  int c = next();
  while ((c >= 'a' && c <= 'z')
         || (c >= 'A' && c <= 'Z')
         || (c >= '0' && c <= '9')
         || c == '_')
  {
    c = next();
  }
  return TokenKind::Id;
}

TokenKind Lexer::lexNumber() {
  int c = next();
  while (c >= '0' && c <= '9') {
    c = next();
  }
  if (c == '.') {
    c = next();
    while (c >= '0' && c <= '9') {
      c = next();
    }
    return TokenKind::Float;
  }
  return TokenKind::Integer;
}

TokenKind Lexer::lexString() {
  int c = next();
  int quote = c;
  while (c != quote) {
    if (c == '\\') {
      next();
    } else if (c == '\r' || c == '\n' || c == 0) {
      return TokenKind::Invalid;
    }

    c = next();
  }
  next();
  return TokenKind::String;
}

TokenKind Lexer::lexLineComment() {
  int c = next();
  while (true) {
    if (c == '\r') {
      if (next() == '\n') {
        next();
      }
      newLine();
      return TokenKind::LineComment;
    } else if (c == '\n') {
      next();
      newLine();
      return TokenKind::LineComment;
    } else if (c == 0) {
      return TokenKind::LineComment;
    } else {
      c = next();
    }
  }
}

TokenKind Lexer::lexBlockComment() {
  next();
  int c = next();
  while (true) {
    if (c == '*') {
      if (next() == '/') {
        next();
        return TokenKind::BlockComment;
      }
    } else if (c == '\r') {
      if ((c = next()) == '\n') {
        c = next();
        newLine();
      }
    } else if (c == '\n') {
      c = next();
      newLine();
    } else if (c == 0) {
      return TokenKind::Invalid;
    } else {
      c = next();
    }
  }
}

TokenKind Lexer::lexOperator() {
  int c;

  switch (current()) {
    default:
      next();
      return TokenKind::Invalid;

    case '~':
      next();
      return TokenKind::Tilde;

    case '@':
      next();
      return TokenKind::At;

    case '#':
      next();
      return TokenKind::Hash;

    case '$':
      next();
      return TokenKind::Dollar;

    case '(':
      next();
      return TokenKind::LParen;

    case ')':
      next();
      return TokenKind::RParen;

    case '[':
      next();
      return TokenKind::LBracket;

    case ']':
      next();
      return TokenKind::RBrace;

    case '{':
      next();
      return TokenKind::LBrace;

    case '}':
      next();
      return TokenKind::RBrace;

    case '\\':
      next();
      return TokenKind::Backslash;

    case ';':
      next();
      return TokenKind::Semi;

    case ',':
      next();
      return TokenKind::Comma;

    case '!':
      if (next() == '=') {
        next();
        return TokenKind::ExclEq;
      }
      return TokenKind::Excl;

    case '%':
      if (next() == '%') {
        next();
        return TokenKind::ModEq;
      }
      return TokenKind::Mod;

    case '^':
      if (next() == '=') {
        next();
        return TokenKind::XorEq;
      }
      return TokenKind::Xor;

    case '=':
      c = next();
      if (c == '=') {
        next();
        return TokenKind::EqEq;
      } else if (c == '>') {
        next();
        return TokenKind::RArrowDbl;
      }
      return TokenKind::Eq;

    case '-':
      c = next();
      if (c == '-') {
        next();
        return TokenKind::MinusMinus;
      } else if (c == '=') {
        next();
        return TokenKind::MinusEq;
      } else if (c == '>') {
        next();
        return TokenKind::RArrowSng;
      }
      return TokenKind::Minus;

    case '&':
      c = next();
      if (c == '&') {
        next();
        return TokenKind::AndAnd;
      } else if (c == '=') {
        next();
        return TokenKind::AndEq;
      }
      return TokenKind::And;

    case '+':
      c = next();
      if (c == '+') {
        next();
        return TokenKind::PlusPlus;
      } else if (c == '=') {
        next();
        return TokenKind::PlusEq;
      }
      return TokenKind::Plus;

    case '*':
      c = next();
      if (c == '*') {
        next();
        return TokenKind::StarStar;
      } else if (c == '=') {
        next();
        return TokenKind::StarEq;
      }

    case '>':
      c = next();
      if (c == '>') {
        if (next() == '=') {
          next();
          return TokenKind::ShrEq;
        }
        return TokenKind::Shr;
      } else if (c == '=') {
        next();
        return TokenKind::GtEq;
      }

    case ':':
      if (next() == ':') {
        next();
        return TokenKind::ColonColon;
      }
      return TokenKind::Colon;

    case '.':
      if (next() == '.') {
        if (next() == '.') {
          next();
          return TokenKind::DotDotDot;
        }
        return TokenKind::DotDot;
      }
      return TokenKind::Dot;

    case '|':
      c = next();
      if (c == '|') {
        next();
        return TokenKind::OrOr;
      } else if (c == '=') {
        next();
        return TokenKind::OrEq;
      } else if (c == '>') {
        next();
        return TokenKind::RTriangle;
      }
      return TokenKind::Or;

    case '<':
      c = next();
      if (c == '<') {
        if (next() == '=') {
          next();
          return TokenKind::ShlEq;
        }
        return TokenKind::Shl;
      } else if (c == '=') {
        if (next() == '>') {
          next();
          return TokenKind::Spaceship;
        }
        return TokenKind::LtEq;
      } else if (c == '|') {
        next();
        return TokenKind::LTriangle;
      } else if (c == '-') {
        next();
        return TokenKind::LArrowSng;
      }
      return TokenKind::Lt;

    case '/':
      if (next() == '=') {
        next();
        return TokenKind::SlashEq;
      }
      return TokenKind::Slash;
  }
}
