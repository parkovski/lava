#include "ash/parser/lexer.h"
#include "ash/terminal/terminal.h"
#include "ash/terminal/ansi.h"

#include <fmt/format.h>
#include <fmt/ostream.h>

#include <string_view>

namespace ash {

int eval(std::string_view code) {
  source::SourceLocator locator;
  auto mainFile = locator.addFile("ash://src/main", code);
  source::Session session(&locator, mainFile);
  parser::Lexer lexer(&session);
  while (true) {
    auto token = lexer();
    fmt::print("> {}\n", token.id);
    if (token.id == parser::Tk::EndOfInput) {
      break;
    }
  }
  return 0;
}

int interactiveMain() {
  if (!term::isTTYInput()) {
    return 1;
  }
  term::initialize();
  term::ScopedRawMode _rawMode{};

  source::SourceLocator locator;
  auto file = locator.addFile("ash://src/shell");
  source::Session session(&locator, file);

  std::string line;
  fmt::print("¿ash? ");
  while (true) {
    int c = term::getChar();
    if (c == 4 && line.empty()) {
      break;
    } else if (c == 13 || c == 10) {
      fmt::print("\r\n¿ash? ");
      line.clear();
    } else if (c == 8 || c == 0x7F) {
      if (!line.empty()) {
        line.erase(line.cend() - 1);
        fmt::print("{} {}", term::ansi::cursor::left(),
                   term::ansi::cursor::left());
      }
    } else {
      putchar(c);
      line.push_back((char)c);
    }
  }

  return 0;
}

} // namespace ash
