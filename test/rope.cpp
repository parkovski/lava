#include "rope/rope.h" 
#include <catch2/catch.hpp>
#include <type_traits>

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

TEST_CASE("Librope additions", "[rope]") {
  rope *r = rope_new_with_utf8_n(u8(g_bigTextBlock), sizeof(g_bigTextBlock) - 1);
  REQUIRE(r);
  //size_t len = rope_char_count(r);
  char buf[5];
  for (size_t i = 0; i < sizeof(buf); ++i) {
    if (buf[i] != g_bigTextBlock[i]) {
      FAIL("rope_new_with_utf8_n miscopied beginning of string");
    }
  }
  for (size_t i = 0; i < sizeof(buf); ++i) {
    if (buf[i] != g_bigTextBlock[sizeof(g_bigTextBlock) - sizeof(buf) + i]) {
      FAIL("rope_new_with_utf8_n miscopied end of string");
    }
  }
  // rope_write_substr
  // rope_write_substr_at_iter
  // rope_insert_n
  // rope_insert_at_iter_n
  rope_free(r);
}

