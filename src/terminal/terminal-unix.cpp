#include "ash/terminal/terminal.h"

#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <csignal>
#include <cassert>
#include <cstdio>
#include <thread>

using namespace ash::term;

static struct termios termAttrs;
static ResizeHandler resizeHandler = nullptr;
static void *resizeParam = nullptr;

static void dispatchSigwinch(int) {
  if (auto h = resizeHandler) {
    auto [width, height] = getScreenSize();
    if (width != 0 && height != 0) {
      h(width, height);
    }
  }
}

void ash::term::initialize() {
  tcgetattr(STDIN_FILENO, &termAttrs);

  ash::term::saveState();

  struct sigaction act;
  sigemptyset(&act.sa_mask);
  act.sa_flags = 0;
  act.sa_handler = &dispatchSigwinch;
  sigaction(SIGWINCH, &act, nullptr);
}

bool ash::term::isTTYInput() {
  return isatty(STDIN_FILENO);
}

bool ash::term::isTTYOutput() {
  return isatty(STDOUT_FILENO);
}

bool ash::term::isTTYError() {
  return isatty(STDERR_FILENO);
}

void ash::term::saveState() {
  tcgetattr(STDIN_FILENO, &termAttrs);
}

void ash::term::setShellState() {
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

void ash::term::restoreState() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termAttrs);
  setvbuf(stdout, nullptr, _IOFBF, BUFSIZ);
}

int ash::term::getChar() {
  return getchar();
}

size_t ash::term::getChars(char *buf, size_t min, size_t max) {
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

std::pair<unsigned short, unsigned short> ash::term::getScreenSize() {
  struct winsize size;
  if (ioctl(STDIN_FILENO, TIOCGWINSZ, &size)) {
    return {0, 0};
  }
  return {size.ws_row, size.ws_col};
}

ResizeHandler ash::term::onResize(ResizeHandler newHandler) {
  auto oldHandler = resizeHandler;
  resizeHandler = newHandler;
  return oldHandler;
}
