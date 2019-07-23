#include "ash/collections/redblacktree.h"

#include <string>

#define CATCH_CONFIG_MAIN
#include <catch.hpp>

using namespace ash::collections::rbtree;

TEST_CASE("Basic rbtree::NodeBase tests", "[rbtree]") {
  struct NodeBaseTest : NodeBase<NodeBaseTest> {
  };

  NodeBaseTest n[3];
  n[0].set_left(&n[1]);
  n[0].set_right(&n[2]);

  REQUIRE(n[0].parent() == nullptr);
  REQUIRE(n[1].parent() == &n[0]);
  REQUIRE(n[2].parent() == &n[0]);
  REQUIRE(n[1].is_left());
  REQUIRE(n[2].is_right());
  REQUIRE(n[1].sibling() == &n[2]);

  n[0].unlink();
  REQUIRE(n[0].left() == nullptr);
  REQUIRE(n[0].right() == nullptr);
}

TEST_CASE("Basic rbtree::NodeKey tests", "[rbtree]") {
  NodeKey<int> int_key(2);
  NodeKey<std::string> string_key(std::string("Hello world"));

  NodeKeyOffset<unsigned, int> offset_key;
  offset_key.set_key(5, 10);
  REQUIRE(offset_key.offset() == -5);
  offset_key.set_offset(5);
  REQUIRE(offset_key.key(10) == 15);
}

TEST_CASE("Basic rbtree::NodeValue tests", "[rbtree]") {
  struct Empty : NodeValue<void> {
    int _bogus;
  };
  REQUIRE(sizeof(Empty) == sizeof(int));

  NodeValue<std::string> str("Hello");
  REQUIRE(str.value() == "Hello");
}

TEST_CASE("Basic rbtree::Node tests", "[rbtree]") {
   Node<bool, bool> bool_node(false, true);
   Node<int, const int> const_int_node(0, 1);

  Node<bool, bool>::const_reference cr = bool_node;
  Node<bool, bool>::reference ref
    = const_node_cast<const Node<bool, bool> &>(cr);
}

TEST_CASE("Basic rbtree::OffsetNode tests", "[rbtree]") {
  using ONode = OffsetNode<unsigned, int, void>;
  ONode n[7];

  // Build up an offset based tree that looks like this:
  //       3
  //    1     5
  //  0   2 4   6

  n[0].set_offset(3);
  n[0].set_left(&n[1]);
    n[1].set_offset(-2);
    n[1].set_left(&n[2]);
      n[2].set_offset(-1);
    n[1].set_right(&n[3]);
      n[3].set_offset(1);
  n[0].set_right(&n[4]);
    n[4].set_offset(2);
    n[4].set_left(&n[5]);
      n[5].set_offset(-1);
    n[4].set_right(&n[6]);
      n[6].set_offset(1);

  // Now make sure the pointer and reference wrappers report the correct types.
  ONode::reference ref(n[0]);
  REQUIRE(ref.key() == 3);
  ref = *ref.left();
  REQUIRE(ref.key() == 1);
  ref = *ref.left();
  REQUIRE(ref.key() == 0);
  ref = *ref.sibling();
  REQUIRE(ref.key() == 2);
  ref = *ref.parent()->sibling();
  REQUIRE(ref.key() == 5);
  ref = *ref.left();
  REQUIRE(ref.key() == 4);
  ref = *ref.sibling();
  REQUIRE(ref.key() == 6);

  // Try moving a couple nodes through the reference wrappers and make sure
  // the offsets update appropriately.
  auto six = ref;
  auto five = *ref.parent();
  auto four = *five.left();
  ref = ONode::reference(n[0]);

  // Try a left rotation at five:
  // 3
  //    6
  //   5
  //  4
  ref.set_right(&six);
  six.set_left(&five);
  five.set_right(nullptr);
  REQUIRE(six.key() == 6);
  REQUIRE(six.offset() == 3);
  REQUIRE(five.key() == 5);
  REQUIRE(five.offset() == -1);

  // Now try this layout:
  // 3
  //   4
  //     6
  //    5

  ref.set_right(&four);
  four.set_right(&six);
  five.set_left(nullptr);
  REQUIRE(four.key() == 4);
  REQUIRE(four.offset() == 1);
  REQUIRE(six.key() == 6);
  REQUIRE(six.offset() == 2);
  REQUIRE(five.key() == 5);
  REQUIRE(five.offset() == -1);
}
