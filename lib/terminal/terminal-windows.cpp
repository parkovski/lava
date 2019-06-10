#include "ash/terminal/terminal.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

static HANDLE stdinHandle;
static HANDLE stdoutHandle;
static HANDLE stderrHandle;
static DWORD stdinMode;
static DWORD stdoutMode;

void ash::term::initialize() {
  stdinHandle = GetStdHandle(STD_INPUT_HANDLE);
  stdoutHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  stderrHandle = GetStdHandle(STD_ERROR_HANDLE);

  ash::term::saveState();
}

bool ash::term::isTTYInput() {
  return GetFileType(stdinHandle) == FILE_TYPE_CHAR;
}

bool ash::term::isTTYOutput() {
  return GetFileType(stdoutHandle) == FILE_TYPE_CHAR;
}

bool ash::term::isTTYError() {
  return GetFileType(stderrHandle) == FILE_TYPE_CHAR;
}

void ash::term::saveState() {
  GetConsoleMode(stdinHandle, &stdinMode);
  GetConsoleMode(stdoutHandle, &stdoutMode);
}

void ash::term::setShellState() {
  SetConsoleMode(stdinHandle,
                 ENABLE_QUICK_EDIT_MODE | ENABLE_EXTENDED_FLAGS
                 | ENABLE_VIRTUAL_TERMINAL_INPUT);
  SetConsoleMode(stdoutHandle,
                 ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT
                 | DISABLE_NEWLINE_AUTO_RETURN
                 | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void ash::term::restoreState() {
  SetConsoleMode(stdinHandle, stdinMode);
  SetConsoleMode(stdoutHandle, stdoutMode);
}

int ash::term::getChar() {
  INPUT_RECORD inp;
  DWORD count;
  while (ReadConsoleInputW(stdinHandle, &inp, 1, &count)) {
    if (inp.EventType != KEY_EVENT || !inp.Event.KeyEvent.bKeyDown) {
      // Discard everything but key down for now.
      continue;
    }
    int ch = inp.Event.KeyEvent.uChar.UnicodeChar;
    if (ch == 0) {
      continue;
    }
    return ch;
  }
  return -1;
}
