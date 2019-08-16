#ifndef ASH_TERMINAL_LINEEDITOR_H_
#define ASH_TERMINAL_LINEEDITOR_H_

#include "ansi.h"
#include "../document/document.h"

#include <string>
#include <string_view>

namespace ash::term {

struct SymbolInfo {
  short fgcolor;
  short bgcolor;
};

class LineEditor {
public:
  enum class Status {
    // End of input was signaled (^D).
    Finished,
    // The user canceled the line by (^C).
    Canceled,
    // The input was accepted (Enter).
    Accepted,
    // The prompt needs to be redrawn. The cursor has already been positioned.
    RedrawPrompt,
    // Couldn't read from stdin.
    ReadError,
    // No problems; keep reading.
    Continue,
  };

  // Read a line interactively. Does not show a prompt.
  Status readLine();

  enum class Mode {
    VimInsert,
    VimCommand,
  };

private:
  Status processControlChar(ansi::TermKey key, bool ctrl, bool alt, bool shift);
  Status processPrintChar(char32_t c, bool alt);

  bool fillBuffer();

  void drawLine() const;

public:
  // Move by (x, y) on screen and update the cursor accordingly.
  void moveBy(short x, short y);

  // Move by an offset in the string.
  void moveBy(ptrdiff_t offset);

  // Move to (x, y) on screen and update the cursor if the line overlaps (x, y).
  void moveTo(unsigned short x, unsigned short y);

  // Move to an index in the string and update the cursor.
  void moveTo(size_t pos);

  bool insert(std::string_view text);
  bool replace(ptrdiff_t count, std::string_view text);
  bool erase(ptrdiff_t count);
  void clear();

  // Write a substring into str.
  size_t substr(char *buf, size_t *bufsize, size_t count = std::string::npos)
                const;

  // Length in characters.
  size_t length() const {
    return _doc.char_length();
  }

  // Size in bytes.
  size_t size() const {
    return _doc.byte_length();
  }

  size_t position() const {
    return _pos;
  }

  Mode mode() const {
    return _mode;
  }

  void setMode(Mode mode) {
    _mode = mode;
  }

private:
  static constexpr size_t BufferLength = 64;
  static constexpr size_t MinReadLength = 8;

  doc::Document<SymbolInfo> _doc;
  Mode _mode = Mode::VimInsert;
  char _readbuf[BufferLength];
  unsigned _rbpos = 0;
  unsigned _rbcnt = 0;
  mutable std::string _textbuf;
  size_t _pos = 0;
  unsigned short _xInit;
  unsigned short _yInit;
  unsigned short _x = 0;
  unsigned short _y = 0;
  unsigned short _screenX;
  unsigned short _screenY;
};

} // namespace ash::term

#endif /* ASH_TERMINAL_LINEEDITOR_H_ */

