#include "ash/symboltable.h"
#include "ash/lineeditor.h"

#include <string>
#include <string_view>
#include <vector>
#include <functional>
#include <iostream>
#include <future>

#ifdef _WIN32

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class ConsoleState {
public:
  explicit ConsoleState() noexcept {
    _stdin = GetStdHandle(STD_INPUT_HANDLE);
    _stdout = GetStdHandle(STD_OUTPUT_HANDLE);
    GetConsoleMode(_stdin, &_stdinMode);
    GetConsoleMode(_stdout, &_stdoutMode);
    SetConsoleMode(_stdin, ENABLE_INSERT_MODE | ENABLE_QUICK_EDIT_MODE
                           | ENABLE_EXTENDED_FLAGS
                           | ENABLE_VIRTUAL_TERMINAL_INPUT);
    SetConsoleMode(_stdout, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT
                            | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
  }

  ~ConsoleState() {
    SetConsoleMode(_stdin, _stdinMode);
    SetConsoleMode(_stdout, _stdoutMode);
  }

private:
  HANDLE _stdin;
  HANDLE _stdout;
  DWORD _stdinMode;
  DWORD _stdoutMode;
};

#else // !_WIN32

#include <unistd.h>
#include <termios.h>

class ConsoleState {
public:
  explicit ConsoleState() noexcept {
    tcgetattr(STDIN_FILENO, &_termAttrs);
    struct termios newAttrs = _termAttrs;
    newAttrs.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
    newAttrs.c_oflag |= (ONLCR);
    newAttrs.c_cflag |= (CS8);
    newAttrs.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
    newAttrs.c_cc[VMIN] = 1;
    newAttrs.c_cc[VTIME] = 0;
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &newAttrs);
  }

  ~ConsoleState() {
    tcsetattr(STDIN_FILENO, TCSAFLUSH, &_termAttrs);
  }

private:
  struct termios _termAttrs;
};

#endif // _WIN32

// symbol = index into list of interned strings
// path = "string" of symbols (basic_string<index_t>).
// a path can have data and attributes.
// an attribute is also a path=>data mapping?

int main(int argc, char *argv[]) {
  using namespace ash;
  [[maybe_unused]] ConsoleState _consoleState;

  SymbolTable symtab;

  std::string buf;
  while (std::cin.good()) {
    std::cout << "\033[91m+\033[m ";
    std::getline(std::cin, buf);
    if (buf == "\004") {
      break;
    }
    std::cout << symtab.findOrInsertSymbol(buf).id() << "\n";
    buf.clear();
  }

  return 0;
}
