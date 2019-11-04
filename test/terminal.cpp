#include "ash/terminal.h"
#include "ash/terminal/ansi.h"

#include <catch2/catch.hpp>

using namespace ash::term;

TEST_CASE("ANSI decoder works", "[terminal]") {
  using namespace ansi;

  DecodeResult r;

  r = decode(std::string_view{"\0junk", 5});
  REQUIRE(r.kind == DecodeControlChar);
  REQUIRE(r.key == TermKey::Nul);
  REQUIRE(r.length == 1);

  r = decode("\tjunk");
  REQUIRE(r.kind == DecodeControlChar);
  REQUIRE(r.key == TermKey::Tab);
  REQUIRE(r.length == 1);

  r = decode("\033[");
  REQUIRE(r.kind == DecodePrintChar);
  REQUIRE(r.alt);
  REQUIRE(r.ch == '[');
  REQUIRE(r.length == 2);

  r = decode("\033\02");
  REQUIRE(r.kind == DecodeControlChar);
  REQUIRE(r.key == TermKey::CtrlB);
  REQUIRE(r.alt);
  REQUIRE(r.length == 2);

  r = decode("\033OA");
  REQUIRE(r.kind == DecodeControlChar);
  REQUIRE(r.key == TermKey::Up);

  r = decode("\033OZ");
  REQUIRE(r.ch == 'O');
  REQUIRE(r.alt);

  r = decode("\033[Z");
  REQUIRE(r.kind == DecodeControlChar);
  REQUIRE(r.key == TermKey::ShiftTab);
  REQUIRE(r.length == 3);

  r = decode("\033[1;6H");
  REQUIRE(r.key == TermKey::Home);
  REQUIRE((r.control & ~r.alt & r.shift));

  r = decode("\033[11~");
  REQUIRE(r.key == TermKey::F1);
  REQUIRE(!(r.control | r.alt | r.shift));

  r = decode("\033[11;8~");
  REQUIRE(r.key == TermKey::F1);
  REQUIRE((r.control & r.alt & r.shift));

  r = decode("\033[11;");
  REQUIRE(r.kind == DecodePartial);

  r = decode("\033[11;H");
  REQUIRE(r.kind == DecodeInvalid);

  r = decode("\033[39;103R");
  REQUIRE(r.kind == DecodeCursorPos);
  REQUIRE(r.pt.x == 103);
  REQUIRE(r.pt.y == 39);
  REQUIRE(r.length == 9);
}
