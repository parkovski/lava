#include "ash/collections/intervaltree.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <iostream>

using ash::collections::IntervalTree;

template<typename Tree>
void print(const Tree &tree) {
  for (auto const it : tree) {
    std::cout << *it << " | ";
  }
  std::cout << "\n";
}

int main() {
  IntervalTree<const char *> tree;

#define MAKE_PAIR(a, b) #a ", " #b
#define INSERT(a, b) tree.insert(a, b, MAKE_PAIR(a, b))
  INSERT(1, 5);
  INSERT(2, 3);
  INSERT(4, 7);
  INSERT(3, 9);
  INSERT(2, 4);
  INSERT(1, 9);
  INSERT(4, 5);
  INSERT(2, 6);
  INSERT(8, 9);
  INSERT(5, 8);
  INSERT(5, 9);
  INSERT(1, 2);

  print(tree);

  tree.erase(tree.find_equal(4, 7));
  tree.erase(tree.find_inner(3, 10));
  tree.erase(tree.find_outer(6, 7));
  tree.erase(tree.find_overlap(1, 2));

  print(tree);

  return 0;
}
