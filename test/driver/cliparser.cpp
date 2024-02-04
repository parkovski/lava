#include <catch2/catch_test_macros.hpp>
#include <catch2/catch_tostring.hpp>
#include "lava/driver/cliparser.h"

using namespace lava::driver;
using namespace std::string_view_literals;

struct TestParser : CliParser {
  using CliParser::CliParser;

protected:
  int apply_short(char arg, bool more, int &argi) noexcept override {
    if (argn == 1) {
      REQUIRE((argi >= 1 && argi <= 3));
      switch (argi) {
      case 1:
        REQUIRE(arg == 'a');
        REQUIRE(more);
        break;

      case 2:
        REQUIRE(arg == 'b');
        REQUIRE(more);
        break;

      case 3:
        REQUIRE(arg == 'c');
        REQUIRE(!more);
        break;
      }
    } else if (argn == 2) {
      REQUIRE(argi == 1);
      REQUIRE(arg == 'v');
      REQUIRE(more);
      std::string_view value;
      REQUIRE(this->value_short(value, argi));
      REQUIRE(argn == 2);
      REQUIRE(value == "value");
    } else if (argn == 3) {
      REQUIRE(argi == 1);
      REQUIRE(!more);
      std::string_view value;
      REQUIRE(this->value_short(value, argi));
      REQUIRE(argn == 4);
      REQUIRE(value == "value");
    } else if (argn == 5) {
      REQUIRE(argi == 0);
      REQUIRE(!more);
      REQUIRE(arg == '-');
    } else {
      FAIL("apply_short called unexpectedly for arg "sv << argn);
    }
    return 0;
  }

  int apply_long(std::string_view arg, std::string_view value) noexcept override {
    if (argn == 6) {
      REQUIRE(arg == "long");
      REQUIRE(value.empty());
      REQUIRE(value_long(value));
      REQUIRE(argn == 7);
      REQUIRE(value == "value");
    } else if (argn == 8) {
      REQUIRE(arg == "long");
      REQUIRE(value == "value");
      REQUIRE(value_long(value));
      REQUIRE(argn == 8);
      REQUIRE(value == "value");
    } else {
      FAIL("apply_long called unexpectedly for arg "sv << argn);
    }
    return 0;
  }

  int apply_other(std::string_view arg) noexcept override {
    if (argn == 9) {
      REQUIRE(arg == "other");
    } else if (argn == 11) {
      REQUIRE(arg == "-other");
    } else {
      FAIL("apply_other called unexpectedly for arg "sv << argn);
    }
    return 0;
  }
};

TEST_CASE("CLI Parser", "[cli]") {
  const char *const argv[] = {
    "appname",
    "-abc",
    "-vvalue",
    "-v", "value",
    "-",
    "--long", "value",
    "--long=value",
    "other",
    "--",
    "-other"
  };
  TestParser parser{sizeof(argv) / sizeof(argv[0]), argv};
  REQUIRE(parser() == 0);
}
