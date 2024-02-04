#include "lava/lava.h"
#include "lava/util/scope_exit.h"
#include "rope/rope.h" 

#include <catch2/catch_test_macros.hpp>

#include <type_traits>
#include <random>
#include <memory>

template<typename Ch>
static constexpr uint8_t *u8(Ch *str) {
  return (uint8_t *)(str);
}

template<typename Ch>
static constexpr const uint8_t *u8(const Ch *str) {
  return (const uint8_t *)(str);
}

static const char g_bigTextBlock[] =
  "Uh, yo, check my flow.\n"
  "I'm the swiftest mad scientist in at least a 10 block radius.\n"
  "Yeah, well, I'll just be waiting till you try to unwind it,\n"
  "I done it big so don't go trying to find it.\n"
  "I'm writing college but your classes' so high school,\n"
  "Got dep injection so my test cases pass, fool.\n"
  "Yeah what you looking at I vim it up all day all night,\n"
  "Getting real tired of trying to make this dumb rap sound alright.\n"
  "Just need a bunch of text to run some sick test cases,\n"
  "And nobody really cares if the rest of it rhymes or not, and I actually "
  "am getting pretty tired of trying to come up with rhymes, so this is it "
  "dudes and such forth and so and so, this is the song, you can go home "
  "now. Seriously. Go. Nothing more here.";

// Some cyrillic letters that don't resemble the Latin alphabet.
static const char g_cyrillic[] =
  "БбГгДдЖжЗзИиЙйЛлПпФфЦцШшЩщЪъЫыЬьЭэЮюЯя";

TEST_CASE("Librope additions", "[rope]") {
  rope *r = rope_new_with_utf8_n(u8(g_bigTextBlock), sizeof(g_bigTextBlock) - 1);
  // rope *r = rope_new_with_utf8(u8(g_bigTextBlock));
  REQUIRE(r);
  LAVA_SCOPE_EXIT { rope_free(r); };

  char smallbuf[6];
  size_t _bytes = sizeof(smallbuf) - 1;
  rope_write_substr(r, u8(smallbuf), &_bytes, 0, _bytes);
  smallbuf[sizeof(smallbuf) - 1] = 0;
  for (size_t i = 0; i < sizeof(smallbuf) - 1; ++i) {
    if (smallbuf[i] != g_bigTextBlock[i]) {
      FAIL("rope_new_with_utf8_n miscopied beginning of string: expected 'Uh, y'; got '"
            << smallbuf << "'.");
    }
  }
  _bytes = sizeof(smallbuf) - 1;
  rope_write_substr(r, u8(smallbuf), &_bytes,
                    sizeof(g_bigTextBlock) - sizeof(smallbuf),
                    sizeof(smallbuf) - 1);
  smallbuf[sizeof(smallbuf) - 1] = 0;
  for (size_t i = 0; i < sizeof(smallbuf) - 1; ++i) {
    if (smallbuf[i] != g_bigTextBlock[sizeof(g_bigTextBlock) - sizeof(smallbuf) + i]) {
      FAIL("rope_new_with_utf8_n miscopied end of string: expected 'here.'; got '"
           << smallbuf << "'.");
    }
  }

  std::random_device rnd;
  for (int i = 0; i < 20; i++) {
    // Note: All cyrillic chars are 2 bytes in utf-8.
    size_t start = (rnd() % sizeof(g_cyrillic)) & (size_t)(-2);
    size_t bytes = (rnd() % (sizeof(g_cyrillic) - start)) & (size_t)(-2);
    size_t index = (rnd() % rope_char_count(r)) & (size_t)(-2);
    rope_insert_n(r, index, u8(g_cyrillic) + start, bytes);
  }

  const size_t charcnt = rope_char_count(r);
  const size_t bytecnt = rope_byte_count(r);
  auto buf = std::make_unique<char[]>(bytecnt + 1);
  buf[bytecnt] = 0;
  for (int i = 0; i < 100; i++) {
    size_t bytes = bytecnt;
    size_t start = rnd() % (charcnt - 1);
    size_t chars = rnd() % (charcnt - start);
    size_t result= rope_write_substr(r, u8(buf.get()), &bytes, start, chars);
    if (result != chars) {
      FAIL("#" << i << ": rope_write_substr(" << start << ", " << chars
           << ") returned " << result);
    }
  }
}

