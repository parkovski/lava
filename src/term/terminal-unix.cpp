#include "lava/term/terminal.h"

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <csignal>
#include <cassert>
#include <cstdio>
#include <thread>

namespace {
  static struct termios termAttrs;
  static lava::term::ResizeHandler resizeHandler = nullptr;
  //static void *resizeParam = nullptr;

  static void dispatchSigwinch(int) {
    if (auto h = resizeHandler) {
      auto size = lava::term::getScreenSize();
      if (size.x && size.y) {
        h(size);
      }
    }
  }
} // anonymous namespace

namespace lava::term {

void initialize() {
  tcgetattr(STDIN_FILENO, &termAttrs);

  saveState();

  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = &dispatchSigwinch;
  sigaction(SIGWINCH, &act, nullptr);
}

bool isTTYInput() {
  return isatty(STDIN_FILENO);
}

bool isTTYOutput() {
  return isatty(STDOUT_FILENO);
}

bool isTTYError() {
  return isatty(STDERR_FILENO);
}

void saveState() {
  tcgetattr(STDIN_FILENO, &termAttrs);
}

void setShellState() {
  struct termios newAttrs = termAttrs;
  newAttrs.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  newAttrs.c_oflag |= (ONLCR);
  newAttrs.c_cflag |= (CS8);
  newAttrs.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  newAttrs.c_cc[VMIN] = 1;
  newAttrs.c_cc[VTIME] = 0;
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &newAttrs);
  setvbuf(stdout, nullptr, _IONBF, 0);
}

void restoreState() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termAttrs);
  setvbuf(stdout, nullptr, _IOFBF, BUFSIZ);
}

int getChar() {
  return getchar();
}

size_t getChars(char *buf, size_t min, size_t max) {
  ssize_t amt;
  size_t total = 0;
  while ((amt = read(STDIN_FILENO, buf, max)) > 0) {
    total += amt;
    buf += amt;
    max -= amt;
    if (total >= min) {
      break;
    }
  }
  return total;
}

Point getScreenSize() {
  struct winsize size;
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &size)) {
    return {0, 0};
  }
  return Point{size.ws_row, size.ws_col};
}

ResizeHandler onResize(ResizeHandler newHandler) {
  auto oldHandler = resizeHandler;
  resizeHandler = newHandler;
  return oldHandler;
}

} // namespace lava::term
