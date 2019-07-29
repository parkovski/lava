#define CATCH_CONFIG_MAIN
#include <catch.hpp>

#include <iostream>
#include "ash/collections/slidingorderedset.h"

TEST_CASE("Foo", "[foo]") {
  using namespace ash::collections::soset;
  SlidingOrderedSet<> set;
  set.insert(50);
  set.insert(30);
  set.insert(90);
  set.insert(20);
  set.insert(10);
  set.insert(60);
  set.insert(70);
  set.insert(40);
  set.insert(80);
  set.insert(0);
  set.insert(100);

  for (auto i = set.begin(), end = set.end(); i != end; ++i) {
    REQUIRE(*i == set.index_for(i) * 10);
  }

  REQUIRE(*set.lower_bound(15) == 20);
  REQUIRE(*set.upper_bound(20) == 30);
  set.shift(75, 10);
  REQUIRE(set.find(110) != set.end());
  set.shift(50, -20);
  REQUIRE(set.find(60) == set.end());
  REQUIRE(set.index_for(set.find(70)) == 6);
  set.insert(60);
  REQUIRE(set.index_for(set.find(60)) == 6);
  REQUIRE(set.index_for(set.find(70)) == 7);
}
