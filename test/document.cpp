#include "ash/ash.h"
#include "ash/document.h"

static inline std::ostream &
operator<<(std::ostream &os, const std::pair<size_t, size_t> &pair) {
  return os << "(" << pair.first << ", " << pair.second << ")";
}

#include <catch2/catch.hpp>

#include <string>
#include <ostream>
#include <cstring>

static constexpr size_t operator""_sz(unsigned long long value) {
  return static_cast<size_t>(value);
}

TEST_CASE("Document handles UTF-8", "[document]") {
  ash::doc::DocumentBase doc;

  // RU bytes = 19
  // EN bytes = 11
  // PT bytes = 10
  const char *const hello_str = "Привет мир\nHello world\nOlá mundo\n";
  constexpr size_t hello_chars = 33;
  constexpr size_t hello_bytes = 43;

  doc.append(hello_str);
  REQUIRE(doc.length() == hello_chars);
  //REQUIRE(doc.size() == hello_bytes);
  REQUIRE(doc.lines() == 4);
  REQUIRE(doc.spanForLine(1) == std::make_pair(0_sz, 10_sz));
  REQUIRE(doc.spanForLine(2) == std::make_pair(11_sz, 22_sz));
  REQUIRE(doc.spanForLine(3) == std::make_pair(23_sz, 32_sz));
  REQUIRE(doc.spanForLine(4) == std::make_pair(33_sz, 33_sz));

  char buffer[256];
  size_t byte_cnt = sizeof(buffer);
  size_t char_cnt = doc.c_substr(buffer, &byte_cnt, 0);
  REQUIRE(char_cnt == hello_chars);
  REQUIRE(byte_cnt == hello_bytes);

  doc.erase(6, 4); // " мир", bytes = 7.
  byte_cnt = sizeof(buffer);
  char_cnt = doc.c_substr(buffer, &byte_cnt, 0);
  REQUIRE(char_cnt == 29);
  REQUIRE(byte_cnt == 36);
  REQUIRE(doc.length() == char_cnt);
  REQUIRE(doc.size() == byte_cnt);
  REQUIRE(strcmp(buffer, "Привет\nHello world\nOlá mundo\n") == 0);
  REQUIRE(doc.spanForLine(1) == std::make_pair(0_sz, 6_sz));
  REQUIRE(doc.spanForLine(2) == std::make_pair(7_sz, 18_sz));
  REQUIRE(doc.spanForLine(3) == std::make_pair(19_sz, 28_sz));
  REQUIRE(doc.spanForLine(4) == std::make_pair(29_sz, 29_sz));

  doc.erase(12, 6); // " world", bytes = 6.
  byte_cnt = sizeof(buffer);
  char_cnt = doc.c_substr(buffer, &byte_cnt, 0);
  REQUIRE(char_cnt == 23);
  REQUIRE(byte_cnt == 30);
  REQUIRE(doc.length() == char_cnt);
  REQUIRE(doc.size() == byte_cnt);
  REQUIRE(strcmp(buffer, "Привет\nHello\nOlá mundo\n") == 0);
  REQUIRE(doc.spanForLine(1) == std::make_pair(0_sz, 6_sz));
  REQUIRE(doc.spanForLine(2) == std::make_pair(7_sz, 12_sz));
  REQUIRE(doc.spanForLine(3) == std::make_pair(13_sz, 22_sz));
  REQUIRE(doc.spanForLine(4) == std::make_pair(23_sz, 23_sz));

  doc.erase(16, 6); // " mundo", bytes = 6.
  byte_cnt = sizeof(buffer);
  char_cnt = doc.c_substr(buffer, &byte_cnt, 0);
  REQUIRE(char_cnt == 17);
  REQUIRE(byte_cnt == 24);
  REQUIRE(doc.length() == char_cnt);
  REQUIRE(doc.size() == byte_cnt);
  REQUIRE(strcmp(buffer, "Привет\nHello\nOlá\n") == 0);
  REQUIRE(doc.spanForLine(1) == std::make_pair(0_sz, 6_sz));
  REQUIRE(doc.spanForLine(2) == std::make_pair(7_sz, 12_sz));
  REQUIRE(doc.spanForLine(3) == std::make_pair(13_sz, 16_sz));
  REQUIRE(doc.spanForLine(4) == std::make_pair(17_sz, 17_sz));
}
