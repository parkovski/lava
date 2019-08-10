#ifndef ASH_TERMINAL_ANSI_H_
#define ASH_TERMINAL_ANSI_H_

#include <cstddef>
#include <cstdint>
#include <string_view>

namespace ash::term {
  struct Point {
    short x;
    short y;
  };

  // What kind of information does this character or sequence provide?
  enum AnsiDecodeResultKind : uint8_t {
    // Unrecognized sequence.
    AnsiInvalid     = 0,
    // Incomplete sequence.
    AnsiPartial     = 1,
    // A printable key.
    AnsiPrintChar   = 2,
    // A non-printable (control) character.
    AnsiControlChar = 3,
    // A cursor position response.
    AnsiCursorPos   = 4,
    // A mouse position response.
    AnsiMousePos    = 5,
  };

  // Terminal control keys.
  enum class TermKey : int {
    None = -1,
    Nul = 0,
    CtrlSpace = 0,
    CtrlAt = 0,
    CtrlA,
    CtrlB,
    CtrlC,
    CtrlD,
    CtrlE,
    CtrlF,
    CtrlG,
    CtrlH,
    Backspace1 = CtrlH,
    CtrlI,
    Tab = CtrlI,
    CtrlJ,
    ShiftEnter = CtrlJ,
    CtrlK,
    CtrlL,
    CtrlM,
    Enter = CtrlM,
    CtrlN,
    CtrlO,
    CtrlP,
    CtrlQ,
    CtrlR,
    CtrlS,
    CtrlT,
    CtrlU,
    CtrlV,
    CtrlW,
    CtrlX,
    CtrlY,
    CtrlZ,
    CtrlLBracket,
    Escape = CtrlLBracket,
    CtrlBackslash,
    CtrlRBracket,
    CtrlCaret,
    Ctrl_,
    ShiftTab,
    Left,
    Right,
    Up,
    Down,
    Insert,
    Delete,
    Home,
    End,
    PageUp,
    PageDown,
    F1,
    F2,
    F3,
    F4,
    F5,
    F6,
    F7,
    F8,
    F9,
    F10,
    F11,
    F12,
    F13,
    F14,
    F15,
    F16,
    F17,
    F18,
    F19,
    F20,
    F21,
    F22,
    F23,
    F24,
    Backspace2 = 0x7F,
  };

  struct AnsiDecodeResult {
    // Kind of information decoded.
    uint8_t kind    : 4;
    // Is there a control modifier?
    uint8_t control : 1;
    // Is there an alt modifier?
    uint8_t alt     : 1;
    // Is there a shift modifier?
    uint8_t shift   : 1;
    // Reserved.
    uint8_t _reserv : 1;
    // How many bytes does this sequence take up?
    uint8_t length;
    // Reserved.
    short _reserv2;
    union {
      // Terminal key if kind is AnsiControlChar.
      TermKey key;
      // Character if kind is AnsiPrintChar.
      char ch;
      // Mouse or cursor position.
      Point pt;
    };
  };

  AnsiDecodeResult decodeAnsi(std::string_view str);
}

#endif // ASH_TERMINAL_ANSI_H_
