#include <utility>

template<typename K>
class RedBlackTree {
  class Node {
    /// Left child, LSB = color.
    Node *_left;
    /// Right child.
    Node *_right;
    /// Key.
    K _key;

  public:
    Node(K &&key, bool red) noexcept(noexcept(K(std::move(key))))
      : _left(nullptr), _right(nullptr), _key(std::move(key))
    {
      if (red) {
        reinterpret_cast<size_t>(_left) = 1;
      }
    }

    Node(const K &key, bool red) noexcept(noexcept(K(key)))
      : _left(nullptr), _right(nullptr), _key(key)
    {
      if (red) {
        reinterpret_cast<size_t>(_left) = 1;
      }
    }

    bool red() const {
      return reinterpret_cast<size_t>(_left) & 1;
    }

    void color(bool red) {
      if (red) {
        reinterpret_cast<size_t>(_left) |= 1;
      } else {
        reinterpret_cast<size_t>(_left) &= ~(size_t)1;
      }
    }

    const Node *left() const {
      return reinterpret_cast<const Node *>(
        reinterpret_cast<size_t>(_left) & ~(size_t)1
      );
    }

    Node *&left() {
      return reinterpret_cast<Node *>(
        reinterpret_cast<size_t>(_left) & ~(size_t)1
      );
    }

    const Node *right() const {
      return _right;
    }

    Node *&right() {
      return _right;
    }

    const K &key() const {
      return _key;
    }

    K &key() {
      return _key;
    }
  };

public:
  void foo() {}
};

#include <iostream>
int main() {
  RedBlackTree<int> tree;
  tree.foo();
  std::cout << "sup\n";
  return 0;
}
