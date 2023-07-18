#include "lava/syn/lexer.h"

using namespace lava::syn;

Lexer::Lexer(const SourceDoc &doc) noexcept
  : doc{&doc}
  , text{doc.content}
  , loc{}
{}

Token Lexer::lex() {
  Token token;
  token.doc = doc;
  token.start = loc;
  int c = getch();
  if (c == -1) {
    token.end = loc;
    token.what = TkEof;
    return token;
  }

  int lower = c | 0x20;
  if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
    lex_whitespace(token);
  } else if (c == '/') {
    int c1 = getch(1);
    if (c1 == '/') {
      lex_line_comment(token);
    } else if (c1 == '*') {
      lex_block_comment(token);
    } else {
      lex_symbol_or_invalid(token);
    }
  } else if ((c >= '0' && c <= '9') || c == '.') {
    lex_number(token);
  } else if (c == '\'' || c == '"') {
    lex_string(token);
  } else if ((lower >= 'a' && lower <= 'z') || lower == '_') {
    lex_ident(token);
    token.end = loc;
    token.what = get_keyword(token.text());
    return token;
  } else {
    lex_symbol_or_invalid(token);
  }

  token.end = loc;
  return token;
}

int Lexer::getch(unsigned lookahead) const {
  if (loc.offset + lookahead >= text.length()) {
    return -1;
  }
  return text[loc.offset + lookahead];
}

void Lexer::nextch() {
  if (text[loc.offset] == '\n') {
    loc.column = 1;
    ++loc.line;
  } else {
    ++loc.column;
  }
  ++loc.offset;
}

int Lexer::get_keyword(std::string_view word) {
  switch (word[0]) {
  case 'e':
    if (word.substr(1) == "lse") {
      return TkElse;
    }
    break;

  case 'f':
    if (word.substr(1) == "un") {
      return TkFun;
    }
    break;

  case 'i':
    if (word.substr(1) == "f") {
      return TkIf;
    }
    break;
  }

  return TkIdent;
}

void Lexer::lex_whitespace(Token &token) {
  token.what = TkWhitespace;
  while (true) {
    nextch();
    int c = getch();
    switch (c) {
    case ' ': case '\t': case '\r': case '\n':
      continue;
    default:
      return;
    }
  }
}

void Lexer::lex_line_comment(Token &token) {
  token.what = TkLineComment;
  nextch();
  nextch();
  while (true) {
    int c = getch();
    switch (c) {
    case -1:
      return;
    case '\n':
      nextch();
      return;
    default:
      nextch();
      break;
    }
  }
}

void Lexer::lex_block_comment(Token &token) {
  token.what = TkBlockComment;
  nextch();
  nextch();
  int c = getch();
  int c1 = getch(1);
  while (true) {
    if (c == '*' && c1 == '/') {
      nextch();
      nextch();
      return;
    } else if (c == -1) {
      // TODO error
      return;
    } else {
      nextch();
      c = c1;
      c1 = getch(1);
    }
  }
}

void Lexer::lex_number(Token &token) {
  int c = getch();
  if (c == '0') {
    int c1 = getch(1);
    int c1l = c1 | 0x20;
    if (c1l == 'x') {
      lex_hex_number(token);
      return;
    } else if (c1l == 'b') {
      lex_binary_number(token);
      return;
    } else if (c1 >= '0' && c1 <= '9') {
      // warn confusion with C octal literals
    }
  } else if (c == '.') {
    int c1 = getch(1);
    if (c1 >= '0' && c1 <= '9') {
      lex_decimal_part(token);
      return;
    } else if (c1 == '.') {
      nextch();
      nextch();
      if (getch() == '.') {
        nextch();
        token.what = TkDotDotDot;
      } else {
        token.what = TkDotDot;
      }
      return;
    } else {
      nextch();
      token.what = TkDot;
      return;
    }
  }

  while (c >= '0' && c <= '9') {
    nextch();
    c = getch();
  }
  if (c == '.') {
    lex_decimal_part(token);
    return;
  }

  token.what = TkIntLiteral;
}

void Lexer::lex_hex_number(Token &token) {
  int c2 = getch(2);
  int c2l = c2 | 0x20;
  if (!((c2 >= '0' && c2 <= '9') || (c2l >= 'a' && c2l <= 'f'))) {
    nextch();
    token.what = TkIntLiteral;
    return;
  }

  nextch();
  nextch();
  token.what = TkHexLiteral;
  while (true) {
    int c = getch();
    int cl = c | 0x20;
    if ((c >= '0' && c <= '9') || (cl >= 'a' && cl <= 'f')) {
      nextch();
    } else {
      return;
    }
  }
}

void Lexer::lex_binary_number(Token &token) {
  int c2 = getch(2);
  if (c2 != '0' && c2 != '1') {
    nextch();
    token.what = TkIntLiteral;
    return;
  }

  nextch();
  nextch();
  token.what = TkBinLiteral;
  while (true) {
    int c = getch();
    if (c == '0' || c == '1') {
      nextch();
    } else {
      return;
    }
  }
}

void Lexer::lex_decimal_part(Token &token) {
  nextch();
  int c = getch();
  while (c >= '0' && c <= '9') {
    nextch();
    c = getch();
  }
  // TODO E+-n
  if (c == 'f') {
    nextch();
    token.what = TkFloatLiteral;
  } else {
    token.what = TkDoubleLiteral;
  }
}

void Lexer::lex_string(Token &token) {
  token.what = TkStringLiteral;
  int close = getch();
  nextch();
  int c = getch();
  while (true) {
    if (c == -1) {
      // TODO error
      return;
    } else if (c == close) {
      nextch();
      return;
    } else {
      nextch();
      c = getch();
    }
  }
}

void Lexer::lex_ident(Token &token) {
  // Assigned in keyword handling.
  // token.what = TkIdent;
  nextch();
  while (true) {
    int c = getch();
    int cl = c | 0x20;
    if ((cl >= 'a' && cl <= 'z') || (cl >= '0' && cl <= '9') || cl == '_') {
      nextch();
    } else {
      return;
    }
  }
}

void Lexer::lex_symbol_or_invalid(Token &token) {
  int c = getch();
  switch (c) {
  case '~':
    nextch();
    token.what = TkTilde;
    break;

  case '!':
    nextch();
    if (getch() == '=') {
      nextch();
      token.what = TkExclEq;
    } else {
      token.what = TkExcl;
    }
    break;

  case '%':
    nextch();
    if (getch() == '=') {
      nextch();
      token.what = TkPercentEq;
    } else {
      token.what = TkPercent;
    }
    break;

  case '^':
    nextch();
    if (getch() == '=') {
      nextch();
      token.what = TkHatEq;
    } else {
      token.what = TkHat;
    }

  case '&':
    nextch();
    {
      int c = getch();
      if (c == '&') {
        nextch();
        token.what = TkAndAnd;
      } else if (c == '=') {
        nextch();
        token.what = TkAndEq;
      } else {
        token.what = TkAnd;
      }
    }
    break;

  case '*':
    nextch();
    {
      int c = getch();
      if (c == '*') {
        nextch();
        if (getch() == '=') {
          nextch();
          token.what = TkStarStarEq;
        } else {
          token.what = TkStarStar;
        }
      } else if (c == '=') {
        nextch();
        token.what = TkStarEq;
      } else {
        token.what = TkStar;
      }
    }
    break;

  case '(':
    nextch();
    token.what = TkLeftParen;
    break;

  case ')':
    nextch();
    token.what = TkRightParen;
    break;

  case '-':
    nextch();
    {
      int c = getch();
      if (c == '-') {
        nextch();
        token.what = TkMinusMinus;
      } else if (c == '=') {
        nextch();
        token.what = TkMinusEq;
      } else if (c == '>') {
        nextch();
        token.what = TkMinusRightArrow;
      } else {
        token.what = TkMinus;
      }
    }
    break;

  case '+':
    nextch();
    {
      int c = getch();
      if (c == '+') {
        nextch();
        token.what = TkPlusPlus;
      } else if (c == '=') {
        nextch();
        token.what = TkPlusEq;
      } else {
        token.what = TkPlus;
      }
    }
    break;

  case '=':
    nextch();
    {
      int c = getch();
      if (c == '=') {
        nextch();
        token.what = TkEqEq;
      } else if (c == '>') {
        nextch();
        token.what = TkEqRightArrow;
      } else {
        token.what = TkEq;
      }
    }
    break;

  case '|':
    nextch();
    {
      int c = getch();
      if (c == '|') {
        nextch();
        token.what = TkOrOr;
      } else if (c == '=') {
        nextch();
        token.what = TkOrEq;
      } else {
        token.what = TkOr;
      }
    }
    break;

  case '[':
    nextch();
    token.what = TkLeftSquareBracket;
    break;

  case ']':
    nextch();
    token.what = TkRightSquareBracket;
    break;

  case '{':
    nextch();
    token.what = TkLeftBrace;
    break;

  case '}':
    nextch();
    token.what = TkRightBrace;
    break;

  case ';':
    nextch();
    token.what = TkSemi;
    break;

  case ':':
    nextch();
    if (getch() == ':') {
      nextch();
      token.what = TkColonColon;
    } else {
      token.what = TkColon;
    }
    break;

  case '<':
    nextch();
    {
      int c = getch();
      if (c == '<') {
        nextch();
        if (getch() == '=') {
          nextch();
          token.what = TkLessLessEq;
        } else {
          token.what = TkLessLess;
        }
      } else if (c == '=') {
        nextch();
        token.what = TkLessEq;
      } else if (c == '-' && getch(1) == '<') {
        nextch();
        nextch();
        if (getch() == '=') {
          nextch();
          token.what = TkLessMinusLessEq;
        } else {
          token.what = TkLessMinusLess;
        }
      } else {
        token.what = TkLess;
      }
    }
    break;

  case '>':
    nextch();
    {
      int c = getch();
      if (c == '>') {
        nextch();
        if (getch() == '=') {
          nextch();
          token.what = TkGreaterGreaterEq;
        } else {
          token.what = TkGreaterGreater;
        }
      } else if (c == '=') {
        nextch();
        token.what = TkGreaterEq;
      } else if (c == '-' && getch(1) == '>') {
        nextch();
        nextch();
        if (getch() == '=') {
          nextch();
          token.what = TkGreaterMinusGreaterEq;
        } else {
          token.what = TkGreaterMinusGreater;
        }
      } else {
        token.what = TkGreater;
      }
    }
    break;

  case ',':
    nextch();
    token.what = TkComma;
    break;

  case '.':
    nextch();
    if (getch() == '.') {
      nextch();
      if (getch() == '.') {
        nextch();
        token.what = TkDotDotDot;
      } else {
        token.what = TkDotDot;
      }
    } else {
      token.what = TkDot;
    }
    break;

  case '/':
    nextch();
    if (getch() == '=') {
      nextch();
      token.what = TkSlashEq;
    } else {
      token.what = TkSlash;
    }
    break;

  case '?':
    nextch();
    token.what = TkQuestion;
    break;

  default:
    nextch();
    token.what = TkInvalid;
    break;
  }
}
