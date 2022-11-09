#include <catch2/catch_test_macros.hpp>
#include "lava/data/arena.h"
#include "lava/util/scope_exit.h"

#define IS_ALIGNED(V,A) (((size_t)(V) & ((size_t)(A) - 1)) == 0)

TEST_CASE("Arena allocator", "[arena]") {
  lava_arena arena;
  lava_arena_init(&arena);

  void *one_page = lava_arena_alloc(&arena, arena.pagesize, arena.pagesize);
  REQUIRE(IS_ALIGNED(one_page, arena.pagesize));
  for (size_t i = 0; i < arena.pagesize / sizeof(int); ++i) {
    ((int *)one_page)[i] = (int)i;
  }

  void *two_pages = lava_arena_alloc(&arena, arena.pagesize, arena.pagesize * 2);
  REQUIRE(IS_ALIGNED(two_pages, arena.pagesize));
  for (size_t i = 0; i < arena.pagesize * 2 / sizeof(int); ++i) {
    ((int *)two_pages)[i] = (int)i;
  }

  void *small = lava_arena_alloc(&arena, alignof(int), sizeof(int));
  void *small2 = lava_arena_alloc(&arena, alignof(int), sizeof(int));
  REQUIRE(((char *)small2) - ((char *)small) == sizeof(int));

  *(int *)small = ((int *)one_page)[arena.pagesize / sizeof(int) - 1];
  *(int *)small2 = ((int *)two_pages)[arena.pagesize / sizeof(int) + 1];
  REQUIRE((*(int *)small) == (int)(arena.pagesize / sizeof(int)) - 1);
  REQUIRE((*(int *)small2) == (int)(arena.pagesize / sizeof(int)) + 1);
}

#include <memory>
TEST_CASE("C++ arena allocator adapter", "[arena]") {
  lava_arena arena;
  lava_arena_init(&arena);
  LAVA_SCOPE_EXIT { lava_arena_fini(&arena); };

  std::vector<int, lava::data::arena_allocator<int>> vec{&arena};

  for (int i = 0; i < 4096; ++i) {
    vec.push_back(i);
  }

  REQUIRE(arena.count >= 4096 * sizeof(int) / arena.pagesize);
}
