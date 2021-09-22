#include "ash/ash.h"
#include "ash/terminal/terminal.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using namespace ash::term;

static DWORD stdinMode;
static DWORD stdoutMode;
static ResizeHandler resizeHandler = nullptr;
static unsigned short bufWidth, bufHeight;

static void postResize() {
  CONSOLE_SCREEN_BUFFER_INFO sbi;
  if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),
                                 &sbi)) {

    if (sbi.dwSize.X == bufWidth && sbi.dwSize.Y == bufHeight) {
      return;
    }
    bufWidth = sbi.dwSize.X;
    bufHeight = sbi.dwSize.Y;
    if (auto h = resizeHandler) {
      h(Point{bufWidth, bufHeight});
    }
  }
}

void ash::term::initialize() {
  bufWidth = 0;
  bufHeight = 0;
  ash::term::saveState();
  postResize();
}

bool ash::term::isTTYInput() {
  return GetFileType(GetStdHandle(STD_INPUT_HANDLE)) == FILE_TYPE_CHAR;
}

bool ash::term::isTTYOutput() {
  return GetFileType(GetStdHandle(STD_OUTPUT_HANDLE)) == FILE_TYPE_CHAR;
}

bool ash::term::isTTYError() {
  return GetFileType(GetStdHandle(STD_ERROR_HANDLE)) == FILE_TYPE_CHAR;
}

void ash::term::saveState() {
  GetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), &stdinMode);
  GetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), &stdoutMode);
}

void ash::term::setShellState() {
  SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE),
                 ENABLE_WINDOW_INPUT | ENABLE_EXTENDED_FLAGS
                 | ENABLE_VIRTUAL_TERMINAL_INPUT);
  SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE),
                 ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT
                 | DISABLE_NEWLINE_AUTO_RETURN
                 | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
}

void ash::term::restoreState() {
  SetConsoleMode(GetStdHandle(STD_INPUT_HANDLE), stdinMode);
  SetConsoleMode(GetStdHandle(STD_OUTPUT_HANDLE), stdoutMode);
}

int ash::term::getChar() {
  INPUT_RECORD inp;
  DWORD count;
  auto hStdin = GetStdHandle(STD_INPUT_HANDLE);
  while (ReadConsoleInputW(hStdin, &inp, 1, &count)) {
    if (inp.EventType == WINDOW_BUFFER_SIZE_EVENT) {
      postResize();
      continue;
    }
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

size_t ash::term::getChars(char *buf, size_t min, size_t max) {
  INPUT_RECORD ir[32];
  constexpr size_t irLen = ASH_ARRAYLEN(ir);
  DWORD count;
  size_t total = 0;
  auto hStdin = GetStdHandle(STD_INPUT_HANDLE);
  DWORD events;
  if (!GetNumberOfConsoleInputEvents(hStdin, &events)) {
    events = 0;
  }

  do {
    if (events > max) {
      if (max > irLen) {
        events = irLen;
      } else {
        events = static_cast<DWORD>(max);
      }
    } else if (total + events < min) {
      events = DWORD(min - total);
    }

    if (!ReadConsoleInputW(hStdin, ir, events, &count)) {
      break;
    }

    for (DWORD i = 0; i < count; ++i) {
      switch (ir[i].EventType) {
        case KEY_EVENT:
          if (!ir[i].Event.KeyEvent.bKeyDown ||
              !ir[i].Event.KeyEvent.uChar.UnicodeChar) {
            break;
          }
          // TODO: UTF-16 to UTF-8 conversion, taking into account max.
          if (ir[i].Event.KeyEvent.uChar.UnicodeChar > 0x7f) {
            break;
          }
          *buf++ = (char)(ir[i].Event.KeyEvent.uChar.UnicodeChar);
          ++total;
          --max;
          break;

        case MOUSE_EVENT:
          // TODO
          break;

        case WINDOW_BUFFER_SIZE_EVENT:
          postResize();
          break;

        default:
          break;
      }
    }

    if (!GetNumberOfConsoleInputEvents(hStdin, &events)) {
      events = 0;
    }
  } while ((max > 0 && events > 0) || total < min);

  return total;
}

Point ash::term::getScreenSize() {
  postResize();
  return Point{bufWidth, bufHeight};
}

ResizeHandler ash::term::onResize(ResizeHandler newHandler) {
  auto oldHandler = resizeHandler;
  resizeHandler = newHandler;
  postResize();
  return oldHandler;
}
