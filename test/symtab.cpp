#include "ash/ash.h"
#include "ash/sym/symboltable.h"
#include "ash/lexer.h"
#include "ash/lineeditor.h"
#include "ash/terminal.h"

#include <iostream>

using namespace ash;

int main() {
  Lexer lexer;
  LineEditor ed;
  std::string line;

  term::initialize();
  ASH_SCOPEEXIT noexcept { term::restoreState(); };

  while (ed.readLine(line)) {
    lexer.setText(line);
    Token t;
    while ((t = lexer.lex()) != Token::Eof) {
      auto const &p = lexer.currentPosition();
      std::cout << "\n(" << p.line << ":" << p.column << "): "
                << t << ": \"" << lexer.currentText() << "\"\n";
    }
  }
  return 0;
}