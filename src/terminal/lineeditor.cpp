#include "ash/ash.h"
#include "ash/terminal.h"
#include "ash/terminal/lineeditor.h"
#include "ash/terminal/ansi.h"

#include <fmt/format.h>
#include <fmt/ostream.h>
#include <cstdlib>

using namespace ash::term;

namespace {

auto getCursorPos() {
  fmt::print(ansi::cursor::query);
  char buf[16];
  auto count = getChars(buf, 1, sizeof(buf));
  char *bufp = buf;
  while (true) {
    auto r = ansi::decode(std::string_view(bufp, count));
    if (r.kind == ansi::DecodeCursorPos) {
      return r.pt;
    }
    if (r.length == count) {
      break;
    }
    count -= r.length;
    bufp += r.length;
  }
  return ansi::Point{};
}

} // anonymous namespace

LineEditor::Status LineEditor::readLine(std::string_view prompt) {
  _promptPos = getCursorPos();
  fmt::print("{}", prompt);

  std::tie(_screen.x, _screen.y) = getScreenSize();

  if (_doc.length()) {
    drawLine();
  } else {
    _inputPos = getCursorPos();
  }

  if (_keybinding == KB_VimInsert) {
    fmt::print("{}", ansi::cursor::style::line(true));
  } else {
    fmt::print("{}", ansi::cursor::style::block(true));
  }

  while (true) {
    if (_rbcnt == 0) {
      // Nothing left in the read buffer.
      _rbpos = 0;
      _rbcnt = static_cast<unsigned>(getChars(_readbuf, 1, sizeof(_readbuf)));
      if (!_rbcnt) {
        return Status::ReadError;
      }
    }

    auto r = ansi::decode(std::string_view(_readbuf + _rbpos, _rbcnt));
    Status status;
    switch (r.kind) {
      case ansi::DecodePartial:
        if (r.length == _rbcnt) {
          fillBuffer();
          continue;
        } else {
          status = Status::Continue;
        }
        break;

      case ansi::DecodeControlChar:
        status = processControlChar(r.key, r.control, r.alt, r.shift);
        break;

      case ansi::DecodePrintChar:
        status = processPrintChar(r.ch, r.alt);
        break;

      default:
        // Ignore unknown input.
        status = Status::Continue;
        break;
    }

    if (!r.length) {
      ++_rbpos;
      --_rbcnt;
    } else if (r.length > _rbcnt) {
      _rbpos = r.length;
      _rbcnt = 0;
    } else {
      _rbpos += r.length;
      _rbcnt -= r.length;
    }

    switch (status) {
      case Status::RedrawPrompt:
        _promptPos = getCursorPos();
        fmt::print("{}", prompt);
        _inputPos = getCursorPos();
        drawLine();
        break;

      case Status::Continue:
        break;

      default:
        return status;
    }
  }
}

LineEditor::Status LineEditor::processControlChar(ansi::TermKey key,
                                                  bool ctrl, bool alt,
                                                  bool shift) {
  using ansi::TermKey;

  (void)ctrl;
  if (alt || shift) {
    // None of these implemented yet.
    return Status::Continue;
  }

  if (key == TermKey::CtrlD
#ifdef _WIN32
      || key == TermKey::CtrlZ
#endif
      ) {
      // End of input.
      return Status::Finished;
  } else if (key == TermKey::CtrlC) {
    clear();
    return Status::Canceled;
  }

  if (_keybinding == KB_VimInsert) {
    switch (key) {
      case TermKey::Escape:
        _keybinding = KB_VimCommand;
        fmt::print("{}", ansi::cursor::style::block(true));
        break;

      case TermKey::Enter:
        return Status::Accepted;

      case TermKey::CtrlEnter:
        insert("\n");
        break;

      case TermKey::Backspace1:
      case TermKey::Backspace2:
        erase(-1);
        break;

      case TermKey::Up:
        moveBy(0, -1);
        break;

      case TermKey::Down:
        moveBy(0, 1);
        break;

      case TermKey::Right:
        moveBy(1, 0);
        break;

      case TermKey::Left:
        moveBy(-1, 0);
        break;

      case TermKey::CtrlL:
        fmt::print("{}{}", ansi::screen::clear,
                   ansi::cursor::move_to(1, 1));
        return Status::RedrawPrompt;

      default:
        break;
    }
  } else if (_keybinding == KB_VimCommand) {
    switch (key) {
      case TermKey::CtrlK:
        clear();
        fmt::print("{}{}", ansi::cursor::move_to(_inputPos.x, _inputPos.y),
                   ansi::line::clear_right);
        break;

      case TermKey::CtrlL:
        fmt::print("{}{}", ansi::screen::clear,
                   ansi::cursor::move_to(1, 1));
        return Status::RedrawPrompt;

      case TermKey::Backspace1:
      case TermKey::Backspace2:
        moveBy(-1);
        break;

      default:
        break;
    }
  }

  return Status::Continue;
}

LineEditor::Status LineEditor::processPrintChar(char32_t c, bool alt) {
  if (alt) {
    // Not implemented.
    return Status::Continue;
  }

  if (_keybinding == KB_VimCommand) {
    switch (c) {
      case U'h':
        moveBy(-1, 0);
        break;

      case U'j':
        moveBy(0, -1);
        break;

      case U'k':
        moveBy(0, 1);
        break;

      case U'l':
        moveBy(1, 0);
        break;

      case 'i':
        _keybinding = KB_VimInsert;
        fmt::print("{}", ansi::cursor::style::line(true));
        break;

      case 'I':
        moveTo(0);
        _keybinding = KB_VimInsert;
        fmt::print("{}", ansi::cursor::style::line(true));
        break;

      case 'a':
        moveBy(1, 0);
        _keybinding = KB_VimInsert;
        fmt::print("{}", ansi::cursor::style::line(true));
        break;

      case 'A':
        moveTo(_doc.length());
        _keybinding = KB_VimInsert;
        fmt::print("{}", ansi::cursor::style::line(true));
        break;

      case 'w':
        moveBy(10, 0);
        break;

      case 'b':
        moveBy(-10, 0);
        break;

      case 'e':
        moveBy(10, 0);
        break;

      case '0':
        moveTo(0);
        break;

      case '^':
        moveTo(0);
        break;

      case '$':
        moveTo(_doc.length());
        break;

      case 't':
        break;

      case 'T':
        break;

      case 'f':
        break;

      case 'F':
        break;

      case 'x':
        erase(1);
        break;

      case 'X':
        erase(-1);
        break;

      case ' ':
        moveBy(1);
        break;

      default:
        break;
    }
    return Status::Continue;
  } else if (_keybinding == KB_VimInsert) {
  } else {
    return Status::Continue;
  }

  char buf[7];
  auto size = utf32_to_utf8(c, buf);
  buf[size] = 0;
  insert(buf);
  return Status::Continue;
}

bool LineEditor::fillBuffer() {
  if (_rbpos + _rbcnt >= BufferLength - MinReadLength) {
    if (_rbcnt >= BufferLength - MinReadLength) {
      return false;
    }
    memmove(_readbuf + _rbpos, _readbuf, _rbcnt);
    _rbpos = 0;
  }
  auto count = getChars(_readbuf + _rbpos + _rbcnt, 1,
                        BufferLength - _rbpos - _rbcnt);
  _rbcnt += static_cast<unsigned>(count);
  return bool(count);
}

void LineEditor::drawLine() {
  _inputPos = getCursorPos();
  auto size = _doc.size();
  _textbuf.reserve(size + 1);
  _doc.substr(_textbuf.data(), &size, 0, _doc.length());
  fmt::print(std::string_view(_textbuf.data(), size));
  if (_x) {
    if (_y == 0) {
      fmt::print("{}", ansi::cursor::move_to(_inputPos.x + _x, _inputPos.y));
    } else {
      fmt::print("{}", ansi::cursor::move_to(_x + 1, _y + _inputPos.y));
    }
  }
}

void LineEditor::moveBy(short x, short y) {
  moveTo(short(_x) + x + 1, short(_y) + y + 1);
}

void LineEditor::moveBy(ptrdiff_t offset) {
  moveTo(size_t(ptrdiff_t(_pos) + offset));
}

void LineEditor::moveTo(unsigned short x, unsigned short y) {
  size_t col = x, line = y;
  _doc.constrain(line, col);
  _pos = _doc.pointToIndex(line, col);
  _x = (unsigned short)col - 1;
  _y = (unsigned short)line - 1;
  fmt::print("{}", ansi::cursor::move_to(_y == 0 ? _x + _inputPos.x : _x + 1,
                                         _y + _inputPos.y));
}

void LineEditor::moveTo(size_t pos) {
  if (pos > length()) {
    pos = length();
  }
  _pos = pos;
  std::tie(_y, _x) = _doc.indexToPoint(pos);
  --_x;
  --_y;
  auto y = _y + _inputPos.y;
  auto x = _x;
  if (_y == 0) {
    x += _inputPos.x;
  } else {
    ++x;
  }
  fmt::print("{}", ansi::cursor::move_to(x, y));
}

bool LineEditor::insert(std::string_view text) {
  unsigned short len = 0;
  for (size_t i = 0; i < text.length(); i += utf8_codepoint_size(text[i])) {
    ++len;
    if (text[i] == '\n') {
      ++_y;
      _x = 0;
    } else if (text[i] == '\r') {
      _x = 0;
    } else if (text[i] == '\t') {
      _x = _x + 8 - _x % 8;
    } else {
      ++_x;
    }
  }
  _textbuf = text;
  _doc.insert(_pos, _textbuf.c_str());
  if (_pos == _doc.length() || _doc[_pos] == '\n') {
    fmt::print(text);
  } else {
    fmt::print("{}{}", ansi::line::insert_space(len), text);
  }
  _pos += len;
  return true;
}

bool LineEditor::replace(ptrdiff_t count, std::string_view text) {
  if (count > 0 && _pos + size_t(count) <= _doc.length()) {
    _textbuf = text;
    _doc.replace(_pos, size_t(count), _textbuf.c_str());
  } else if (size_t(-count) <= _pos) {
    _textbuf = text;
    _doc.replace(size_t(_pos - count), size_t(-count), _textbuf.c_str());
  } else {
    return false;
  }
  return true;
}

bool LineEditor::erase(ptrdiff_t count) {
  if (count > 0) {
    auto fwdcount = static_cast<size_t>(count);
    if (_pos + fwdcount > _doc.length()) {
      fwdcount = _doc.length() - _pos;
    }
    _doc.erase(_pos, fwdcount);
    fmt::print("{}", ansi::line::delete_space((unsigned short)(fwdcount)));
  } else if (size_t(-count) <= _pos) {
    auto bkwdcount = static_cast<size_t>(-count);
    if (bkwdcount > _pos) {
      bkwdcount = _pos;
    }
    unsigned short bkcount_s = static_cast<unsigned short>(bkwdcount);
    _pos -= bkwdcount;
    if (_x > bkwdcount) {
      _x -= bkwdcount;
      fmt::print("{}{}", ansi::cursor::left(bkcount_s),
                 ansi::line::delete_space(bkcount_s));
    } else {
      moveTo(_pos);
    }
    _doc.erase(_pos, bkwdcount);
  } else {
    return false;
  }
  return true;
}

void LineEditor::clear() {
  _x = 0;
  _y = 0;
  _keybinding = KB_VimInsert;
  _rbpos = 0;
  _rbcnt = 0;
  _pos = 0;
  _doc.clear();
}

size_t LineEditor::substr(char *buf, size_t *bufsize, size_t count) const {
  return _doc.substr(buf, bufsize, _pos, _pos + count);
}
