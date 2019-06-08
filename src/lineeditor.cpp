#include "ash/lineeditor.h"
#include "ash/document.h"
#include "ash/terminal.h"

#include <cstdio>

using namespace ash;

LineEditor::LineEditor() noexcept
  : _prompt{LineEditor::defaultPrompt}
{}

void LineEditor::defaultPrompt() {
  // 90 = gray, 91 = red
  printf("\033[90mash!\033[m ");
}

bool LineEditor::readLine(std::string &line) {
  _prompt();
  line.clear();
  cursorPos = 0;
  cursorLeft = 1;
  cursorTop = 1;

  while (true) {
    int ch = term::getChar();
    if (ch == 4 // ^D
#ifdef _WIN32
        || ch == 26 // ^Z
#endif
        ) {
      // End of input.
      return false;
    } else if (ch == 11) { // ^K
      // Clear the screen.
      printf("\033[2K\033[G");
      _prompt();
    } else if (ch == 12) { // ^L
      // Clear the current line.
      printf("\033[2J\033[H");
      _prompt();
    } else if (ch == '\r' || ch == '\n') {
      putchar('\n');
      return true;
    } else if (ch == 8 || ch == 0x7F) {
      // Backspace
      // Move left and erase the next (current before moving) character.
      // TODO: Check position & don't allow to erase the prompt.
      printf("\033[D\033[X");
      if (line.length()) {
        line.erase(line.end() - 1);
      }
    } else if (ch == -1) {
      return false;
    } else if (ch <= 0) {
      continue;
    } else {
      line.push_back((char)ch);
      putchar(ch);
    }
    //symtab.findOrInsertSymbol(buf).id()
  }
}

