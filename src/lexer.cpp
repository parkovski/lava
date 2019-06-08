#include "ash/lexer.h"

#include <ostream>

using namespace ash;

std::ostream &ash::operator<<(std::ostream &os, Token token) {
  const char *text;
  switch (token) {
    default:
      text = "unknown";
      break;

    case Token::Invalid:
      text = "invalid";
      break;

    case Token::Tilde:
      text = "tilde";
      break;

    case Token::At:
      text = "at";
      break;

    case Token::Dollar:
      text = "dollar";
      break;

    case Token::LParen:
      text = "left parenthesis";
      break;

    case Token::RParen:
      text = "right parenthesis";
      break;

    case Token::LBracket:
      text = "left square bracket";
      break;

    case Token::RBracket:
      text = "right square bracket";
      break;

    case Token::LBrace:
      text = "left brace";
      break;

    case Token::RBrace:
      text = "right brace";
      break;

    case Token::Backslash:
      text = "backslash";
      break;

    case Token::Semi:
      text = "semi";
      break;

    case Token::Comma:
      text = "comma";
      break;

    case Token::Excl:
      text = "exclamation";
      break;

    case Token::ExclEq:
      text = "exclamation equal";
      break;

    case Token::Mod:
      text = "modulo";
      break;

    case Token::ModEq:
      text = "modulo equal";
      break;

    case Token::Xor:
      text = "xor";
      break;

    case Token::XorEq:
      text = "xor equal";
      break;

    case Token::Eq:
      text = "equal";
      break;

    case Token::EqEq:
      text = "double equal";
      break;

    case Token::RArrowDbl:
      text = "right double arrow";
      break;

    case Token::Minus:
      text = "minus";
      break;

    case Token::MinusMinus:
      text = "double minus";
      break;

    case Token::MinusEq:
      text = "minus equal";
      break;

    case Token::RArrowSng:
      text = "right single arrow";
      break;

    case Token::And:
      text = "and";
      break;

    case Token::AndAnd:
      text = "double and";
      break;

    case Token::AndEq:
      text = "and equal";
      break;

    case Token::Plus:
      text = "plus";
      break;

    case Token::PlusPlus:
      text = "double plus";
      break;

    case Token::PlusEq:
      text = "plus equal";
      break;

    case Token::Star:
      text = "star";
      break;

    case Token::StarStar:
      text = "double star";
      break;

    case Token::StarEq:
      text = "star equal";
      break;

    case Token::Gt:
      text = "greater";
      break;

    case Token::Shr:
      text = "shift right";
      break;

    case Token::GtEq:
      text = "greater equal";
      break;

    case Token::ShrEq:
      text = "shift right equal";
      break;

    case Token::Colon:
      text = "colon";
      break;

    case Token::ColonColon:
      text = "double colon";
      break;

    case Token::Dot:
      text = "dot";
      break;

    case Token::DotDot:
      text = "double dot";
      break;

    case Token::DotDotDot:
      text = "triple dot";
      break;

    case Token::Or:
      text = "or";
      break;

    case Token::OrOr:
      text = "double or";
      break;

    case Token::OrEq:
      text = "or equal";
      break;

    case Token::RTriangle:
      text = "right triangle";
      break;

    case Token::Lt:
      text = "less";
      break;

    case Token::Shl:
      text = "shift left";
      break;

    case Token::LtEq:
      text = "less equal";
      break;

    case Token::ShlEq:
      text = "shift left equal";
      break;

    case Token::Spaceship:
      text = "spaceship";
      break;

    case Token::LTriangle:
      text = "left triangle";
      break;

    case Token::LArrowSng:
      text = "left single arrow";
      break;

    case Token::Slash:
      text = "slash";
      break;

    case Token::SlashEq:
      text = "slash equal";
      break;

    case Token::Eof:
      text = "EOF";
      break;

    case Token::Whitespace:
      text = "whitespace";
      break;

    case Token::NewLine:
      text = "new line";
      break;

    case Token::LineComment:
      text = "line comment";
      break;

    case Token::BlockComment:
      text = "block comment";
      break;

    case Token::Id:
      text = "id";
      break;

    case Token::Integer:
      text = "integer";
      break;

    case Token::Float:
      text = "float";
      break;

    case Token::String:
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
    return Token::Eof;
  }
  _prev = _cur;

  auto c = current();
  if (c == ' ' || c == '\t') {
    return lexWhitespace();
  } else if (c == '\r' || c == '\n') {
    return lexNewLine();
  } else if ((c >= 'a' && c <= 'z')
             || (c >= 'A' && c <= 'Z')
             || c == '_')
  {
    return lexId();
  } else if (c >= '0' && c <= '9') {
    return lexNumber();
  } else if (c == '\'' || c == '"' || c == '`') {
    return lexString();
  }
  return lexOperator();
}

Token Lexer::lexWhitespace() {
  int c = current();
  while (c == ' ' || c == '\t') {
    c = next();
  }
  return Token::Whitespace;
}

Token Lexer::lexNewLine() {
  if (current() == '\r') {
    if (next() == '\n') {
      next();
    }
    newLine();
  } else if (current() == '\n') {
    next();
    newLine();
  }
  return Token::NewLine;
}

Token Lexer::lexId() {
  int c = next();
  while ((c >= 'a' && c <= 'z')
         || (c >= 'A' && c <= 'Z')
         || (c >= '0' && c <= '9')
         || c == '_')
  {
    c = next();
  }
  return Token::Id;
}

Token Lexer::lexNumber() {
  int c = next();
  while (c >= '0' && c <= '9') {
    c = next();
  }
  if (c == '.') {
    c = next();
    while (c >= '0' && c <= '9') {
      c = next();
    }
    return Token::Float;
  }
  return Token::Integer;
}

Token Lexer::lexString() {
  int c = next();
  int quote = c;
  while (c != quote) {
    if (c == '\\') {
      next();
    } else if (c == '\r' || c == '\n' || c == 0) {
      return Token::Invalid;
    }

    c = next();
  }
  next();
  return Token::String;
}

Token Lexer::lexLineComment() {
  int c = next();
  while (true) {
    if (c == '\r') {
      if (next() == '\n') {
        next();
      }
      newLine();
      return Token::LineComment;
    } else if (c == '\n') {
      next();
      newLine();
      return Token::LineComment;
    } else if (c == 0) {
      return Token::LineComment;
    } else {
      c = next();
    }
  }
}

Token Lexer::lexBlockComment() {
  next();
  int c = next();
  while (true) {
    if (c == '*') {
      if (next() == '/') {
        next();
        return Token::BlockComment;
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
      return Token::Invalid;
    } else {
      c = next();
    }
  }
}

Token Lexer::lexOperator() {
  int c;

  switch (current()) {
    default:
      next();
      return Token::Invalid;

    case '~':
      next();
      return Token::Tilde;

    case '@':
      next();
      return Token::At;

    case '#':
      next();
      return Token::Hash;

    case '$':
      next();
      return Token::Dollar;

    case '(':
      next();
      return Token::LParen;

    case ')':
      next();
      return Token::RParen;

    case '[':
      next();
      return Token::LBracket;

    case ']':
      next();
      return Token::RBrace;

    case '{':
      next();
      return Token::LBrace;

    case '}':
      next();
      return Token::RBrace;

    case '\\':
      next();
      return Token::Backslash;

    case ';':
      next();
      return Token::Semi;

    case ',':
      next();
      return Token::Comma;

    case '!':
      if (next() == '=') {
        next();
        return Token::ExclEq;
      }
      return Token::Excl;

    case '%':
      if (next() == '%') {
        next();
        return Token::ModEq;
      }
      return Token::Mod;

    case '^':
      if (next() == '=') {
        next();
        return Token::XorEq;
      }
      return Token::Xor;

    case '=':
      c = next();
      if (c == '=') {
        next();
        return Token::EqEq;
      } else if (c == '>') {
        next();
        return Token::RArrowDbl;
      }
      return Token::Eq;

    case '-':
      c = next();
      if (c == '-') {
        next();
        return Token::MinusMinus;
      } else if (c == '=') {
        next();
        return Token::MinusEq;
      } else if (c == '>') {
        next();
        return Token::RArrowSng;
      }
      return Token::Minus;

    case '&':
      c = next();
      if (c == '&') {
        next();
        return Token::AndAnd;
      } else if (c == '=') {
        next();
        return Token::AndEq;
      }
      return Token::And;

    case '+':
      c = next();
      if (c == '+') {
        next();
        return Token::PlusPlus;
      } else if (c == '=') {
        next();
        return Token::PlusEq;
      }
      return Token::Plus;

    case '*':
      c = next();
      if (c == '*') {
        next();
        return Token::StarStar;
      } else if (c == '=') {
        next();
        return Token::StarEq;
      }

    case '>':
      c = next();
      if (c == '>') {
        if (next() == '=') {
          next();
          return Token::ShrEq;
        }
        return Token::Shr;
      } else if (c == '=') {
        next();
        return Token::GtEq;
      }

    case ':':
      if (next() == ':') {
        next();
        return Token::ColonColon;
      }
      return Token::Colon;

    case '.':
      if (next() == '.') {
        if (next() == '.') {
          next();
          return Token::DotDotDot;
        }
        return Token::DotDot;
      }
      return Token::Dot;

    case '|':
      c = next();
      if (c == '|') {
        next();
        return Token::OrOr;
      } else if (c == '=') {
        next();
        return Token::OrEq;
      } else if (c == '>') {
        next();
        return Token::RTriangle;
      }
      return Token::Or;

    case '<':
      c = next();
      if (c == '<') {
        if (next() == '=') {
          next();
          return Token::ShlEq;
        }
        return Token::Shl;
      } else if (c == '=') {
        if (next() == '>') {
          next();
          return Token::Spaceship;
        }
        return Token::LtEq;
      } else if (c == '|') {
        next();
        return Token::LTriangle;
      } else if (c == '-') {
        next();
        return Token::LArrowSng;
      }
      return Token::Lt;

    case '/':
      if (next() == '=') {
        next();
        return Token::SlashEq;
      }
      return Token::Slash;
  }
}
