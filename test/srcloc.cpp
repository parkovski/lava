#include "ash/srcloc/sourcelocator.h"

#include <catch2/catch.hpp>

TEST_CASE("SourceLocator basic tests", "[srcloc]") {
  using namespace ash::srcloc;
  using namespace std::string_view_literals;

  SourceLocator lctr;
  auto fileFoo = lctr.addFile("foo.ash"sv);
  auto fileBar = lctr.addFile("bar.ash"sv);

  REQUIRE(lctr.findFile("foo.ash"sv) == fileFoo);
  REQUIRE(lctr.findFile("bar.ash"sv) == fileBar);
  REQUIRE_FALSE(lctr.findFile("does_not_exist"sv).isValid());
  REQUIRE(lctr.fileName(fileFoo) == "foo.ash"sv);
  REQUIRE(lctr.fileName(fileBar) == "bar.ash"sv);
  REQUIRE(lctr.fileName(FileId()).empty());

  SourceLocation fooLocs[] = {
    {fileFoo, 0, 1, 1},
    {fileFoo, 10, 1, 11},
    {fileFoo, 15, 2, 1},
  };
  constexpr size_t fooLocCount = sizeof(fooLocs) / sizeof(fooLocs[0]);

  LocId fooLocIds[fooLocCount];
  for (size_t i = 0; i < fooLocCount; ++i) {
    fooLocIds[i] = lctr.mark(fooLocs[i]);
  }

  SourceLocation barLocs[] = {
    {fileBar, 0, 1, 1},
    {fileBar, 1, 1, 2},
    {fileBar, 2, 2, 1},
    {fileBar, 3, 2, 2},
  };
  constexpr size_t barLocCount = sizeof(barLocs) / sizeof(barLocs[0]);

  LocId barLocIds[barLocCount];
  for (size_t i = 0; i < barLocCount; ++i) {
    barLocIds[i] = lctr.mark(barLocs[i]);
  }

  for (size_t i = 0; i < fooLocCount; ++i) {
    SourceLocation sl = lctr.find(fooLocIds[i]);
    REQUIRE(sl.file == fileFoo);
    REQUIRE(sl.index == fooLocs[i].index);
    REQUIRE(sl.line == fooLocs[i].line);
    REQUIRE(sl.column == fooLocs[i].column);
  }

  for (size_t i = 0; i < barLocCount; ++i) {
    SourceLocation sl = lctr.find(barLocIds[i]);
    REQUIRE(sl.file == fileBar);
    REQUIRE(sl.index == barLocs[i].index);
    REQUIRE(sl.line == barLocs[i].line);
    REQUIRE(sl.column == barLocs[i].column);
  }
}
