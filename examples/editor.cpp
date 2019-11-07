#include "ash/ash.h"
#include "ash/terminal.h"
#include "ash/document.h"

#include <fmt/format.h>

#include <cstdlib>
#include <functional>

using namespace ash;

struct Point {
  int x;
  int y;
};

struct Editor {
  Point size;
  Point cursor = {1, 1};
  Point scroll = {0, 0};
  constexpr static int seqlen = 32;
  char seq[seqlen] = {0};
  int seqidx = -1;
  doc::Document<unsigned> doc;
  size_t charidx = 0;
  constexpr static int buflen = 256;
  char buf[buflen];
  size_t charcnt = 0;
  char lastch = 0;

  constexpr static auto ctrl(int letter) -> int {
    return letter - '@';
  }

  void draw_debug() {
    fmt::print("\033[{};{}H", doc.lines() + 2, 1);
    auto span = doc.spanForLine(cursor.y);
    fmt::print("cur={:03}/{:03} (x={:02}, y={:02}); lns={:02}; l={}:{}, "
               "ch={:2X}, chs={}",
               charidx, doc.length(), cursor.x, cursor.y, doc.lines(),
               span.first, span.second, lastch, charcnt);
    fmt::print("\033[{};{}H", cursor.y, cursor.x);
  }

  void draw_partial_line(size_t line, size_t xoff) {
    auto [c0, c1] = doc.spanForLine(line);
    c0 += xoff;
    // Cursor to line and clear right.
    fmt::print("\033[{};{}H\033[K", cursor.y, 1 + xoff);
    size_t bytes = buflen;
    doc.c_substr(buf, &bytes, c0, c1);
    fmt::print("{}\033[{};{}H", buf, cursor.y, cursor.x);
  }

  void draw_lines(size_t start, size_t count, bool clear) {
    if (start == 0) {
      start = 1;
    }
    if (count == 0) {
      return;
    }
    if (auto lines = doc.lines(); count > lines - start + 1) {
      count = lines - start + 1;
    }
    for (size_t i = start; i < start + count; ++i) {
      // Clear the line.
      if (clear) {
        fmt::print("\033[2K\033[G");
      }
      auto [c0, c1] = doc.spanForLine(i);
      size_t bytes = buflen;
      doc.c_substr(buf, &bytes, c0, c1);
      puts(buf);
    }
  }

  void type(int ch) {
    char s[2] = {(char)ch, 0};
    doc.insert(charidx, s);
    ++charidx;
    putchar(ch);
  }

  bool handle_char(int ch) {
    switch (ch) {
      case -1: // EOF
      case ctrl('C'):
        // Exit
        return false;

      case ctrl('W'):
        // Up
        if (cursor.y > 1) {
          auto [c0, c1] = doc.spanForLine(--cursor.y);
          auto len = c1 - c0;
          if (size_t(cursor.x) > len) {
            cursor.x = (unsigned short)(len + 1);
          }
          charidx = c0 + cursor.x - 1;
          // Move up, horizontal absolute.
          fmt::print("\033[A\033[{}G", cursor.x);
        }
        break;

      case ctrl('A'):
        // Left
        if (cursor.x > 1) {
          --cursor.x;
          --charidx;
          fmt::print("\033[D");
        }
        break;

      case ctrl('S'):
        // Down
        if (size_t(cursor.y) < doc.lines()) {
          auto [c0, c1] = doc.spanForLine(++cursor.y);
          auto len = c1 - c0;
          if (size_t(cursor.x) > len) {
            cursor.x = len + 1;
          }
          charidx = c0 + cursor.x - 1;
          fmt::print("\033[B\033[{}G", cursor.x);
        }
        break;

      case ctrl('D'):
        // Right
        if (size_t(cursor.x) < charidx - doc.spanForLine(cursor.y).second) {
          ++cursor.x;
          ++charidx;
          fmt::print("\033[C");
        }
        break;

      case ctrl('K'):
        // Clear line.
        {
          auto span = doc.spanForLine(cursor.y);
          doc.erase(span.first, span.second);
          charidx = span.first;
          cursor.x = 1;
          fmt::print("\033[2K\033[G");
        }
        break;

      case ctrl('L'):
        // Clear screen.
        fmt::print("\033[2J\033[H");
        draw_lines(1, doc.lines(), false);
        fmt::print("\033[{};{}H", cursor.y, cursor.x);
        break;

      case '\r':
      case '\n':
        // Clear the debug line and the rest of this line.
        fmt::print("\033[{}d\033[2K\033[{};{}H\033[K",
                   doc.lines() + 2, cursor.y, cursor.x);
        cursor.x = 1;
        ++cursor.y;
        type('\n');
        draw_partial_line(cursor.y, 0);
        break;

      case ctrl('H'):
      case 0x7F:
        // Backspace
        if (charidx > 0) {
          if (doc[charidx] == '\n') {
            --cursor.y;
            cursor.x = doc.spanForLine(cursor.y).second;
            // TODO: Remove this line.
            fmt::print("\033[A\033[{}G", cursor.x);
          } else if (cursor.x > 0) {
            --cursor.x;
            draw_partial_line(cursor.y, cursor.x - 1);
          } else {
            break;
          }
          doc.erase(charidx - 1, charidx);
          --charidx;
        }
        break;

      default:
        if (isprint(ch)) {
          ++cursor.x;
          type(ch);
        }
        break;
    }

    lastch = ch;
    ++charcnt;
    draw_debug();
    return true;
  }

  void run() {
    fmt::print("\033[2J\033[H\033[1 q");
    char buf[64];
    size_t count;
    while ((count = term::getChars(buf, 1, 64)) > 0) {
      for (size_t i = 0; i < count; ++i) {
        if (!handle_char(buf[i])) {
          return;
        }
      }
    }
  }
};

int main(int argc, char *argv[]) {
  /*if (!term::isTTYInput()) {
    fmt::print(stderr, "stdin is not a tty!");
    return 1;
  }
  if (!term::isTTYOutput()) {
    fmt::print(stderr, "stdout is not a tty!");
    return 1;
  }*/

  term::initialize();
  term::setShellState();
  // Set alt buffer.
  fmt::print("\033[?1049h\033[2J");

  Editor().run();

  ASH_SCOPEEXIT{
    // End alt buffer.
    fmt::print("\033[?1049l");
    term::restoreState();
  };

  return 0;
}
