#ifndef ASH_TERMINAL_ANSI_H_
#define ASH_TERMINAL_ANSI_H_

#include <cstddef>
#include <cstdint>
#include <string_view>
#include <iosfwd>
#include <tuple>
#include <fmt/format.h>

namespace ash::term::ansi {

struct Point {
  short x;
  short y;
};

// What kind of information does this character or sequence provide?
enum DecodeResultKind : uint8_t {
  // Unrecognized sequence.
  DecodeInvalid     = 0,
  // Incomplete sequence.
  DecodePartial     = 1,
  // A printable key.
  DecodePrintChar   = 2,
  // A non-printable (control) character.
  DecodeControlChar = 3,
  // A cursor position response.
  DecodeCursorPos   = 4,
  // A mouse position response.
  DecodeMousePos    = 5,
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
  CtrlEnter = CtrlJ,
  LF = CtrlJ,
  CtrlK,
  CtrlL,
  CtrlM,
  Enter = CtrlM,
  CR = CtrlM,
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

struct DecodeResult {
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
  uint16_t _reserv2;
  union {
    // Terminal key if kind is DecodeControlChar.
    TermKey key;
    // Character if kind is DecodePrintChar.
    int ch;
    // Mouse or cursor position.
    Point pt;
  };
};

// Decodes an ANSI output sequence emitted by the terminal.
DecodeResult decode(std::string_view str);

// Control sequences.
  namespace format {
    template<typename Tuple, size_t N = std::tuple_size_v<Tuple>>
    struct OutputWrapper {
      explicit OutputWrapper(const Tuple &tuple) noexcept
        : tuple(tuple)
      {}

      friend std::ostream &operator<<(std::ostream &os,
                                      OutputWrapper self) {
        return os << OutputWrapper<Tuple, N-1>(self.tuple)
                  << std::get<N-1>(self.tuple);
      }

      const Tuple &tuple;
    };

    template<typename Tuple>
    struct OutputWrapper<Tuple, 0> {
      explicit OutputWrapper(const Tuple &) noexcept
      {}

      friend std::ostream &operator<<(std::ostream &os,
                                      OutputWrapper) {
        return os;
      }
    };

    template<typename... Args>
    struct Print {
      explicit Print(Args &&...args)
        : tuple(std::forward<Args>(args)...)
      {}

      friend std::ostream &operator<<(std::ostream &os, const Print &self) {
        return os << OutputWrapper<std::tuple<Args...>>(self.tuple);
      }

      std::tuple<Args...> tuple;
    };

    template<typename... Args>
    Print(Args &&...args) -> Print<Args...>;
  } // namespace format

  namespace cursor {
    inline auto up(unsigned short y = 1)
    { return format::Print("\033[", y, "A"); }

    inline auto down(unsigned short y = 1)
    { return format::Print("\033[", y, "B"); }

    inline auto right(unsigned short x = 1)
    { return format::Print("\033[", x, "C"); }

    inline auto left(unsigned short x = 1)
    { return format::Print("\033[", x, "D"); }

    inline auto next_line(unsigned short y = 1)
    { return format::Print("\033[", y, "E"); }

    inline auto prev_line(unsigned short y = 1)
    { return format::Print("\033[", y, "F"); }

    inline auto to_col(unsigned short x)
    { return format::Print("\033[", x, "G"); }

    inline auto to_row(unsigned short y)
    { return format::Print("\033[", y, "d"); }

    inline auto move_to(unsigned short x, unsigned short y)
    { return format::Print("\033[", y, ";", x, "H"); }

    inline auto to_next_tab(unsigned short n)
    { return format::Print("\033[", n, "I"); }

    inline auto to_prev_tab(unsigned short n)
    { return format::Print("\033[", n, "Z"); }

    inline auto blink(bool enable = true)
    { return enable ? "\033[?12h" : "\033[?12l"; }

    inline auto show(bool enable = true)
    { return enable ? "\033[?25h" : "\033[?25l"; }

    constexpr auto save = "\033[s";

    constexpr auto restore = "\033[u";

    constexpr auto query = "\033[6n";

    namespace style {
      inline auto block(bool blink = true)
      { return blink ? "\033[1 q" : "\033[2 q"; }

      inline auto underline(bool blink = true)
      { return blink ? "\033[3 q" : "\033[4 q"; }

      inline auto line(bool blink = true)
      { return blink? "\033[5 q": "\033[6 q"; }
    } // namespace style
  } // namespace cursor

  namespace screen {
    inline auto scroll_up(unsigned short y)
    { return format::Print("\033[", y, "S"); }

    inline auto scroll_down(unsigned short y)
    { return format::Print("\033[", y, "T"); }

    constexpr auto clear_down = "\033[J";

    constexpr auto clear_up = "\033[1J";

    constexpr auto clear = "\033[2J";

    constexpr auto set_tab = "\033H";

    constexpr auto clear_tab = "\033[0g";

    constexpr auto clear_all_tabs = "\033[3g";

    inline auto set_scroll_region(unsigned short top, unsigned short bottom)
    { return format::Print("\033[", top, ";", bottom, "r"); }

    constexpr auto clear_scroll_region = "\033[;r";
  } // namespace screen

  namespace line {
    inline auto insert_space(unsigned short n)
    { return format::Print("\033[", n, "@"); }

    inline auto delete_space(unsigned short n)
    { return format::Print("\033[", n, "P"); }

    constexpr auto clear_right = "\033[0K";

    constexpr auto clear_left = "\033[1K";

    constexpr auto clear = "\033[2K";

    inline auto erase(unsigned short n)
    { return format::Print("\033[", n, "X"); }
  } // namespace line

  namespace alt_buffer {
    constexpr auto enter = "\033[?1049h";

    constexpr auto exit = "\033[?1049l";
  } // namespace alt_buffer

  namespace style {
    template<size_t N = 1>
    struct Style;

    template<>
    struct Style<0> {
      constexpr explicit Style() noexcept
      {}

      friend std::ostream &operator<<(std::ostream &os, const Style<0> &) {
        return os << "\033[m";
      }
    };

    template<>
    struct Style<1> {
      constexpr explicit Style(unsigned short value) noexcept
        : value(value)
      {}

      friend std::ostream &operator<<(std::ostream &os, const Style &self) {
        return os << "\033[" << self.value << "m";
      }

      void write_part(std::ostream &os) const {
        os << value;
      }

      unsigned short value;
    };

    template<size_t N>
    struct Style {
      constexpr explicit Style(Style<N-1> &&styles, unsigned short value)
        noexcept
        : styles(std::move(styles)), value(value)
      {}

      friend std::ostream &operator<<(std::ostream &os, const Style &self) {
        os << "\033[";
        self.styles.write_part(os);
        return os << self.value << "m";
      }

      void write_part(std::ostream &os) const {
        styles.write_part(os);
        os << value << ";";
      }

      Style<N-1> styles;
      unsigned short value;
    };

    inline Style<2> operator+(Style<0>, Style<1> s2) {
      return Style<2>(Style<1>(0), s2.value);
    }

    template<size_t N>
    inline Style<N+1> operator+(Style<N> s1, Style<1> s2) {
      return Style<N+1>(std::move(s1), s2.value);
    }

    constexpr auto clear        = Style<0>();
    constexpr auto bold         = Style<>(1);
    constexpr auto underline    = Style<>(4);
    constexpr auto no_underline = Style<>(24);
    constexpr auto negative     = Style<>(7);
    constexpr auto positive     = Style<>(27);
  } // namespace style

  namespace fg {
    constexpr auto black   = style::Style<>(30);
    constexpr auto red     = style::Style<>(31);
    constexpr auto green   = style::Style<>(32);
    constexpr auto yellow  = style::Style<>(33);
    constexpr auto blue    = style::Style<>(34);
    constexpr auto magenta = style::Style<>(35);
    constexpr auto cyan    = style::Style<>(36);
    constexpr auto white   = style::Style<>(37);
    constexpr auto default_= style::Style<>(39);

    constexpr auto bright_black   = style::Style<>(90);
    constexpr auto bright_red     = style::Style<>(91);
    constexpr auto bright_green   = style::Style<>(92);
    constexpr auto bright_yellow  = style::Style<>(93);
    constexpr auto bright_blue    = style::Style<>(94);
    constexpr auto bright_magenta = style::Style<>(95);
    constexpr auto bright_cyan    = style::Style<>(96);
    constexpr auto bright_white   = style::Style<>(97);
  } // namespace fg

  namespace bg {
    constexpr auto black   = style::Style<>(40);
    constexpr auto red     = style::Style<>(41);
    constexpr auto green   = style::Style<>(42);
    constexpr auto yellow  = style::Style<>(43);
    constexpr auto blue    = style::Style<>(44);
    constexpr auto magenta = style::Style<>(45);
    constexpr auto cyan    = style::Style<>(46);
    constexpr auto white   = style::Style<>(47);
    constexpr auto default_= style::Style<>(49);

    constexpr auto bright_black   = style::Style<>(100);
    constexpr auto bright_red     = style::Style<>(101);
    constexpr auto bright_green   = style::Style<>(102);
    constexpr auto bright_yellow  = style::Style<>(103);
    constexpr auto bright_blue    = style::Style<>(104);
    constexpr auto bright_magenta = style::Style<>(105);
    constexpr auto bright_cyan    = style::Style<>(106);
    constexpr auto bright_white   = style::Style<>(107);
  } // namespace bg

} // namespace ash::term::ansi

#endif // ASH_TERMINAL_ANSI_H_
