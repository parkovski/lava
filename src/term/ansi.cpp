#include "lava/lava.h"
#include "lava/term/ansi.h"

using namespace lava::term::ansi;

// TODO: These are generated on Windows. May need to add more later.
// - ^[Ox - x in [A, B, C, D, H, F, P, Q, R, S]
// - ^[[x - x in [A, B, C, D, H, F, Z]
// - ^[[1;nx - x from above lists (except Z). N designates the following:
//   - 2: Shift
//   - 3: Alt
//   - 4: Alt+Shift
//   - 5: Control
//   - 6: Control+Shift
//   - 7: Control+Alt
//   - 8: Control+Alt+Shift
// - ^[[n~ - n is a 1 or 2 digit number.
// - ^[[n;m~ - n same as above, m is from the above modifier list.
// - ^[[y;xR - cursor position.

namespace {

bool setModifiers(DecodeResult &r, int m) {
  switch (m) {
    case 8:
      r.control = 1;
    case 4:
      r.alt = 1;
    case 2:
      r.shift = 1;
      break;

    case 7:
      r.control = 1;
    case 3:
      r.alt = 1;
      break;

    case 6:
      r.shift = 1;
    case 5:
      r.control = 1;
      break;

    default:
      return false;
  }

  return true;
}

// Form ^[Ox. Sets key only, not length or kind.
bool convertEscO(DecodeResult &r, char c) {
  switch (c) {
    case 'A':
      r.key = TermKey::Up;
      break;

    case 'B':
      r.key = TermKey::Down;
      break;

    case 'C':
      r.key = TermKey::Right;
      break;

    case 'D':
      r.key = TermKey::Left;
      break;

    case 'H':
      r.key = TermKey::Home;
      break;

    case 'F':
      r.key = TermKey::End;
      break;

    case 'P':
      r.key = TermKey::F1;
      break;

    case 'Q':
      r.key = TermKey::F2;
      break;

    case 'R':
      r.key = TermKey::F3;
      break;

    case 'S':
      r.key = TermKey::F4;
      break;

    default:
      return false;
  }

  return true;
}

// Form ^[[x where x is a letter. Sets result fields on success only.
bool convertEscBLetter(DecodeResult &r, char c) {
  if (c >= 'P' && c <= 'S') {
    return false;
  }
  if (c == 'Z') {
    r.kind = DecodeControlChar;
    r.key = TermKey::ShiftTab;
    r.shift = true;
    r.length = 3;
    return true;
  }
  if (convertEscO(r, c)) {
    r.kind = DecodeControlChar;
    r.length = 3;
    return true;
  }
  return false;
}

bool setEscBNumKey(DecodeResult &r, int key) {
  switch (key) {
    case 1:
      r.key = TermKey::Home;
      break;

    case 2:
      r.key = TermKey::Insert;
      break;

    case 3:
      r.key = TermKey::Delete;
      break;

    case 4:
      r.key = TermKey::End;
      break;

    case 5:
      r.key = TermKey::PageUp;
      break;

    case 6:
      r.key = TermKey::PageDown;
      break;

    case 11:
      r.key = TermKey::F1;
      break;

    case 12:
      r.key = TermKey::F2;
      break;

    case 13:
      r.key = TermKey::F3;
      break;

    case 14:
      r.key = TermKey::F4;
      break;

    case 15:
      r.key = TermKey::F5;
      break;

    case 17:
      r.key = TermKey::F6;
      break;

    case 18:
      r.key = TermKey::F7;
      break;

    case 19:
      r.key = TermKey::F8;
      break;

    case 20:
      r.key = TermKey::F9;
      break;

    case 21:
      r.key = TermKey::F10;
      break;

    case 22:
      r.key = TermKey::F11;
      break;

    case 23:
      r.key = TermKey::F12;
      break;

    case 24:
      r.key = TermKey::F13;
      break;

    case 25:
      r.key = TermKey::F14;
      break;

    case 26:
      r.key = TermKey::F15;
      break;

    case 27:
      r.key = TermKey::F16;
      break;

    case 28:
      r.key = TermKey::F17;
      break;

    case 29:
      r.key = TermKey::F18;
      break;

    case 30:
      r.key = TermKey::F19;
      break;

    case 31:
      r.key = TermKey::F20;
      break;

    case 32:
      r.key = TermKey::F21;
      break;

    case 33:
      r.key = TermKey::F22;
      break;

    case 34:
      r.key = TermKey::F23;
      break;

    case 35:
      r.key = TermKey::F24;
      break;

    default:
      return false;
  }

  return true;
}

// Forms ^[[x... where x is a number.
// Where m is a ctrl-alt-shift modifier number, key is a key from the ^[O list,
// n is a control key identifier, and x and y are coordinates:
// - ^[[1;mk
// - ^[[n~
// - ^[[y;xR
// - ^[[n;m~
bool convertEscBNum(DecodeResult &r, const char *s, const char *end) {
  int a = 0, b = 0;
  r.length = 2;

  while (s < end && *s >= '0' && *s <= '9') {
    a *= 10;
    a += *s - '0';
    ++s;
    ++r.length;
  }
  if (s == end) {
    r.kind = DecodePartial;
    return true;
  }

  if (*s == '~') {
    ++r.length;
    return setEscBNumKey(r, a);
  }

  if (*s++ == ';') {
    ++r.length;
  } else {
    return false;
  }

  if (s == end) {
    r.kind = DecodePartial;
    return true;
  }

  while (s < end && *s >= '0' && *s <= '9') {
    b *= 10;
    b += *s - '0';
    ++s;
    ++r.length;
  }
  if (s == end) {
    r.kind = DecodePartial;
    return true;
  }

  if (*s == '~') {
    if (!setEscBNumKey(r, a) || !setModifiers(r, b)) {
      ++r.length;
      return false;
    }
  } else if (*s == 'R') {
    r.kind = DecodeCursorPos;
    r.pt.y = static_cast<short>(a);
    r.pt.x = static_cast<short>(b);
  } else if (a == 1 && convertEscO(r, *s)) {
    if (!setModifiers(r, b)) {
      return false;
    }
  } else {
    return false;
  }

  ++r.length;
  return true;
}

} // anonymous namespace

DecodeResult lava::term::ansi::decode(std::string_view str) {
  DecodeResult result{};
  result.key = TermKey::None;
  auto const length = str.length();
  if (length == 0) {
    result.kind = DecodeInvalid;
    return result;
  }

  const char *s = str.data();
  const char *end = s + length;
  // Escape sequence or alt+key.
  if (*s == '\033' && length > 1) {
    ++s;
    if (length > 2) {
      if (*s == 'O') {
        ++s;
        if (convertEscO(result, *s)) {
          result.kind = DecodeControlChar;
          result.length = 3;
        } else {
          result.kind = DecodePrintChar;
          result.alt = 1;
          result.ch = 'O';
          result.length = 2;
        }
        return result;
      } else if (*s == '[') {
        ++s;
        if (convertEscBLetter(result, *s)) {
        } else if (*s >= '0' && *s <= '9') {
          if (!convertEscBNum(result, s, end)) {
            result.kind = DecodeInvalid;
          }
        } else {
          result.kind = DecodePrintChar;
          result.alt = 1;
          result.ch = '[';
          result.length = 2;
        }
        return result;
      }
    }
    // Alt+something...
    result.alt = 1;
    result.length = 1;
  }

  // Control keys ^@ to ^_
  if (*s >= '\0' && *s <= '\x1F') {
    result.control = 1;
    result.kind = DecodeControlChar;
    result.key = static_cast<TermKey>(*s);
    ++result.length;
  } else if (*s == '\x7f') {
    result.control = 1;
    result.kind = DecodeControlChar;
    result.key = TermKey::Backspace2;
    ++result.length;
  } else {
    result.kind = DecodePrintChar;
    result.length += static_cast<uint8_t>(utf8_codepoint_size(*s));
    if (static_cast<ptrdiff_t>(result.length) < end - s) {
      result.length += static_cast<uint8_t>(end - s);
      result.kind = DecodePartial;
    }
    result.ch = utf8_to_utf32(s);
  }
  return result;
}
