#include "ash/collections/intervaltree.h"

#include <iostream>

using ash::collections::IntervalTree;

template<typename It, typename End>
void print(const char *label, It it, End end) {
  std::cout << label;
  for (; it != end; ++it) {
    std::cout << " | " << it->start_pos() << ", " << it->end_pos()
              << " (" << **it << ")";
  }
  std::cout << "\n";
}

using ash::collections::itree::Key;
template<typename T>
void printnode(Key<T> key) {
  std::cout << key.start_pos() << "," << key.end_pos() << "/";
  if (key.node()->offset() > 0) {
    std::cout << "+";
  }
  std::cout << key.node()->offset();
  if (!key.left() && !key.right()) {
    return;
  }
  std::cout << " [";
  if (key.left()) {
    printnode(key.left());
  } else {
    std::cout << "null";
  }
  std::cout << "; ";
  if (key.right()) {
    printnode(key.right());
  } else {
    std::cout << "null";
  }
  std::cout << "]";
}

template<typename It>
void printtree(It it) {
  auto node = it->node();
  while (node->parent()) {
    node = node->parent();
  }
  printnode(Key<const char *const>(node, node->offset()));
  std::cout << "\n";
}

int main() {
  IntervalTree<const char *> tree;

#define MAKE_PAIR(a, b) #a ", " #b
#define INSERT(a, b) tree.insert(a, b, MAKE_PAIR(a, b)); printtree(tree.begin())
  INSERT(1, 5);
  INSERT(2, 4);
  INSERT(4, 7);
  INSERT(3, 9);
  INSERT(1, 9);
  INSERT(4, 5);
  INSERT(8, 9);
  INSERT(5, 8);
  INSERT(5, 9);

  print("Tree   ", tree.begin(), tree.end());
  //printtree(tree.begin());

  //print("= 4, 7 ", tree.find_equal(4, 7), tree.end());
  //print("[5, 10]", tree.find_inner(5, 10), tree.end());
  //print("]6, 7[ ", tree.find_outer(6, 7), tree.end());
  //print("|1, 2| ", tree.find_overlap(1, 2), tree.end());

  print("|  5  | ", tree.find(5), tree.end());
  tree.shift(5, 5);
  //print("Tree    ", tree.begin(), tree.end());
  printtree(tree.begin());
  print("[12, 15]", tree.find_inner(12, 15), tree.end());
  print("|12, 15|", tree.find_overlap(12, 15), tree.end());
  tree.shift(12, -3);
  print("Tree   ", tree.begin(), tree.end());

  /*
  tree.erase(tree.find_equal(4, 7));   // 4,7
  tree.erase(tree.find_inner(3, 10));  // 3,9
  //tree.erase(tree.find_outer(6, 7));   // 1,9
  tree.erase(tree.find_overlap(1, 2)); // 1,5

  print("Tree   ", tree.begin(), tree.end());

  */
  tree.shift(0, 2);
  print("Tree   ", tree.begin(), tree.end());

  return 0;
}
