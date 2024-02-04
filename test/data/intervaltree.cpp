#include <catch2/catch_test_macros.hpp>
#include "lava/data/intervaltree.h"

TEST_CASE("Interval tree", "[data][intervaltree]") {
  using namespace lava::data::itree;

  IntervalTree<std::string_view> tree;
#define MAKE_PAIR(a, b) #a ", " #b
#define INSERT(a, b) tree.insert(a, b, MAKE_PAIR(a, b))
  INSERT(1, 5);
  INSERT(2, 4);
  INSERT(4, 7);
  INSERT(3, 9);
  INSERT(1, 9);
  INSERT(4, 5);
  INSERT(8, 9);
  INSERT(5, 8);
  INSERT(5, 9);
#undef INSERT
#undef MAKE_PAIR

  int count = 0;
  for (auto i = tree.find(5); i != tree.end(); ++i) {
    ++count;
    REQUIRE(i->start_pos() <= 5);
    REQUIRE(i->end_pos() >= 5);
  }
  REQUIRE(count == 5);

  tree.shift(5, 5);
  auto inner = tree.find_inner(12, 15);
  REQUIRE(inner->start_pos() == 13);
  REQUIRE(inner->end_pos() == 14);
  REQUIRE(++inner == tree.end());

  count = 0;
  for (auto i = tree.find_overlap(12, 15); i != tree.end(); ++i) {
    ++count;
    REQUIRE(i->start_pos() < 15);
    REQUIRE(i->end_pos() > 12);
  }
  REQUIRE(count == 5);

  tree.shift(12, -3);
  count = 0;
  for (auto i = tree.find_equal(5, 12); i != tree.end(); ++i) {
    ++count;
  }
  REQUIRE(count == 2);
  tree.shift(0, 2);
  REQUIRE(tree.find(1) == tree.end());
}
