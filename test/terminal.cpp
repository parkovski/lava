#include "ash/terminal/ansi.h"

#include <catch2/catch.hpp>

using namespace ash::term;

TEST_CASE("ANSI decoder works", "[terminal]") {
  AnsiDecodeResult r;

  r = decodeAnsi(std::string_view{"\0junk", 5});
  REQUIRE(r.kind == AnsiControlChar);
  REQUIRE(r.key == TermKey::Nul);
  REQUIRE(r.length == 1);

  r = decodeAnsi("\tjunk");
  REQUIRE(r.kind == AnsiPrintChar);
  REQUIRE(r.ch == '\t');
  REQUIRE(r.length == 1);

  r = decodeAnsi("\033[");
  REQUIRE(r.kind == AnsiPrintChar);
  REQUIRE(r.alt);
  REQUIRE(r.ch == '[');
  REQUIRE(r.length == 2);

  r = decodeAnsi("\033\02");
  REQUIRE(r.kind == AnsiControlChar);
  REQUIRE(r.key == TermKey::CtrlB);
  REQUIRE(r.alt);
  REQUIRE(r.length == 2);

  r = decodeAnsi("\033OA");
  REQUIRE(r.kind == AnsiControlChar);
  REQUIRE(r.key == TermKey::Up);

  r = decodeAnsi("\033OZ");
  REQUIRE(r.ch == 'O');
  REQUIRE(r.alt);

  r = decodeAnsi("\033[Z");
  REQUIRE(r.kind == AnsiControlChar);
  REQUIRE(r.key == TermKey::ShiftTab);
  REQUIRE(r.length == 3);

  r = decodeAnsi("\033[1;6H");
  REQUIRE(r.key == TermKey::Home);
  REQUIRE((r.control & ~r.alt & r.shift));

  r = decodeAnsi("\033[11~");
  REQUIRE(r.key == TermKey::F1);
  REQUIRE(!(r.control | r.alt | r.shift));

  r = decodeAnsi("\033[11;8~");
  REQUIRE(r.key == TermKey::F1);
  REQUIRE((r.control & r.alt & r.shift));

  r = decodeAnsi("\033[11;");
  REQUIRE(r.kind == AnsiPartial);

  r = decodeAnsi("\033[11;H");
  REQUIRE(r.kind == AnsiInvalid);

  r = decodeAnsi("\033[39;103R");
  REQUIRE(r.kind == AnsiCursorPos);
  REQUIRE(r.pt.x == 103);
  REQUIRE(r.pt.y == 39);
  REQUIRE(r.length == 9);
}
