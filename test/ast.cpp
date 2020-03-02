#include "ash/ast/node.h"

#include <catch2/catch.hpp>

#include <vector>
#include <memory>

template<typename T>
using IteratorWrapper = ash::ast::detail::SmartPtrIteratorWrapper<T>;

TEST_CASE("SmartPtrIteratorWrapper", "[ast]") {
  auto wrap_iter = [](auto iter) {
    return ash::ast::detail::SmartPtrIteratorWrapper<decltype(iter)>(iter);
  };
  std::vector<std::unique_ptr<int>> vec;
  std::generate_n(std::back_inserter(vec), 5, [](){
    static int i = 0; return std::make_unique<int>(++i);
  });

  {
    auto const &const_vec = vec;
    auto it = wrap_iter(const_vec.begin());
    auto end = wrap_iter(const_vec.end());
    for (int i = 1; it != end; ++it, ++i) {
      REQUIRE(*it == i);
    }
  }

  {
    auto it = wrap_iter(vec.begin());
    auto end = wrap_iter(vec.end());
    for (; it != end; ++it) {
      *it *= 2;
    }
  }

  {
    auto it = wrap_iter(vec.rbegin());
    auto end = wrap_iter(vec.rend());
    for (int i = 10; it != end; ++it, i -= 2) {
      REQUIRE(*it == i);
      *it -= 1;
    }
  }

  {
    auto const &const_vec = vec;
    auto it = wrap_iter(const_vec.rbegin());
    auto end = wrap_iter(const_vec.rend());
    for (int i = 9; it != end; ++it, i -= 2) {
      REQUIRE(*it == i);
    }
  }

  {
    auto cbegin = wrap_iter(vec.cbegin());
    auto cend = wrap_iter(vec.cend());
    auto crbegin = wrap_iter(vec.crbegin());
    auto crend = wrap_iter(vec.crend());
    REQUIRE((cend - cbegin) == 5);
    REQUIRE((crbegin - crend) == -5);
    REQUIRE(cbegin[1] == 3);
    REQUIRE(crbegin[1] == 7);
    REQUIRE(*++cbegin == 3);
    REQUIRE(*--cend == 9);
    REQUIRE((cbegin + 1) == (cend - 2));
    REQUIRE(*(cbegin + 1) == 5);
    REQUIRE(*++crbegin == 7);
    REQUIRE(*--crend == 1);
    REQUIRE((crbegin + 1) == (crend - 2));
    REQUIRE(*(crbegin + 1) == 5);
  }
}

TEST_CASE("AST NodeList", "[ast]") {
  using namespace ash::ast;
  NodeList nodes;
  REQUIRE(nodes.empty());
}
