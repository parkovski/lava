#include "ash/collections/redblacktree.h"

using ash::collections::RedBlackTree;

int main() {
  RedBlackTree<int> foo;

  foo.insert(1);
  foo.insert(5);
  foo.insert(10);
  foo.insert(3);
  foo.insert(5);
  foo.insert(24);
  foo.insert(18);
  foo.insert(12);
  foo.insert(0);

  ++foo.begin();
}
