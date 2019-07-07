#include "ash/collections/intervaltree.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

using ash::collections::IntervalTree;

int main() {
  IntervalTree<int> tree;

  tree.insert(1, 5, 1050);
  tree.insert(2, 3, 2030);
  tree.insert(4, 7, 4070);
  tree.insert(3, 9, 3090);
  tree.insert(2, 4, 2040);
  tree.insert(1, 9, 1090);
  tree.insert(4, 5, 4050);
  tree.insert(2, 6, 2060);
  tree.insert(8, 9, 8090);
  tree.insert(5, 8, 5080);
  tree.insert(5, 9, 5090);
  tree.insert(1, 2, 1020);

  tree.erase(tree.find_equal(4, 7));
  tree.erase(tree.find_inner(3, 10));
  tree.erase(tree.find_outer(6, 7));
  tree.erase(tree.find_overlap(1, 2));

  return 0;
}
