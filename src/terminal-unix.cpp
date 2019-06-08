#include "ash/terminal.h"

#include <unistd.h>
#include <termios.h>

static struct termios termAttrs;

void ash::term::initialize() {
  tcgetattr(STDIN_FILENO, &termAttrs);

  ash::term::saveState();
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
}

void ash::term::restoreState() {
  tcsetattr(STDIN_FILENO, TCSAFLUSH, &termAttrs);
}

int ash::term::getChar() {
  return getchar();
}
