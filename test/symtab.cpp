#include "ash/ash.h"
#include "ash/terminal/terminal.h"
#include "ash/terminal/lineeditor.h"
#include "ash/parser/lexer.h"
#include "ash/parser/parser.h"
#include "ash/symbol/symboltable.h"

#include <iostream>

using namespace ash;

int main() {
  term::initialize();
  ASH_SCOPEEXIT noexcept { term::restoreState(); };

  LineEditor ed;
  std::string line;
  Lexer lexer;
  Parser parser;

  while (ed.readLine(line)) {
    lexer.setText(line);
    Token t;
    do {
      Token t = lexer.lex();
      auto const &p = lexer.currentPosition();
      std::cout << "\n(" << p.line << ":" << p.column << "): "
                << t.kind << ": \"" << lexer.currentText() << "\"\n";
    } while (t.kind != TokenKind::Eof);
  }
  return 0;
}