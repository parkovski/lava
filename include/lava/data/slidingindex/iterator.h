#ifndef LAVA_DATA_SLIDINGINDEX_ITERATOR_H_
#define LAVA_DATA_SLIDINGINDEX_ITERATOR_H_

#include "node.h"

#include <iterator>
#include <type_traits>
#include <utility>
#include <cassert>

namespace lava::data::slidx {

// This iterator meets all the requirements of a random access iterator
// _except_ arbitrary movement is not constant time.
template<typename P, typename O>
class Iterator {
public:
  typedef ptrdiff_t difference_type;
  typedef P value_type;
  typedef const value_type *pointer;
  typedef const value_type &reference;
  typedef std::bidirectional_iterator_tag iterator_category;

  template<typename, typename, typename>
  friend class SlidingIndex;

  // Empty iterator constructor.
  Iterator() = default;

  // Constructor for the root. The root's index and position are known to be
  // its left subtree's size and its offset, respectively.
  explicit Iterator(Node<P, O> *root) noexcept
    : _node(root), _index(root->left() ? root->left()->size() : 0),
      _value(root->offset())
  {}

  explicit Iterator(Node<P, O> *node, size_t index, P value)
    : _node(node), _index(index), _value(value)
  {}

  // Copy constructor.
  Iterator(const Iterator &) = default;

  // Copy assignment.
  Iterator &operator=(const Iterator &other) = default;

  // Equality comparison by node pointers.
  bool operator==(const Iterator &other) const {
    assert(!_node || !other._node || (_node == other._node) ==
           (_index == other._index && _value == other._value));
    return _node == other._node;
  }

  bool operator!=(const Iterator &other) const {
    return !(*this == other);
  }

  // Less than by index.
  bool operator<(const Iterator &other) const {
    return _value < other._value;
  }

  bool operator<=(const Iterator &other) const {
    return !(other < *this);
  }

  bool operator>(const Iterator &other) const {
    return other < *this; 
  }

  bool operator>=(const Iterator &other) const {
    return !(*this < other);
  }

  // Move this iterator forward or backward by distance. Distance may be equal
  // to the size of the whole tree - the current index, in which case this
  // becomes a past-the-end iterator. Behavior is undefined if the target index
  // is outside the range [0, tree size].
  Iterator &operator+=(difference_type distance) {
    size_t target = _index + distance;
    while (distance != 0) {
      Node<P, O> *right;
      Node<P, O> *left;

      if (distance > 0 && (right = _node->right()) &&
          static_cast<size_t>(distance) <= right->size()) {
        // Target node is in the right subtree.
        --distance;
        _node = right;
        _value += _node->offset();
        if ((left = _node->left())) {
          distance -= left->size();
        }
      } else if (distance < 0 && (left = _node->left()) &&
                 static_cast<size_t>(-distance) <= left->size()) {
        // Target node is in the left subtree.
        ++distance;
        _node = left;
        _value += _node->offset();
        if ((right = _node->right())) {
          distance += right->size();
        }
      } else if (_node->parent()) {
        // Target node is outside of this subtree.
        if (_node->is_left()) {
          // Parent index is greater than this node.
          --distance;
          if ((right = _node->right())) {
            distance -= right->size();
          }
        } else {
          // Parent index is less than this node.
          ++distance;
          if ((left = _node->left())) {
            distance += left->size();
          }
        }
        _value -= _node->offset();
        _node = _node->parent();
      } else {
        // No parent. This becomes a past-the-end iterator.
        target = _node->size();
        _node = nullptr;
        break;
      }
    }
    _index = target;
    return *this;
  }

  // Create an iterator offset from this by distance.
  friend Iterator operator+(const Iterator &it, difference_type distance) {
    Iterator next = it;
    return next += distance;
  }

  // Create an iterator offset from this by distance.
  friend Iterator operator+(difference_type distance, const Iterator &it) {
    return it + distance;
  }

  // Move the iterator forward or backward by -distance.
  Iterator &operator-=(difference_type distance) {
    return *this += -distance;
  }

  // Create an iterator offset from this by -distance.
  friend Iterator operator-(const Iterator &it, difference_type distance) {
    Iterator next = it;
    return next += -distance;
  }

  // Distance between two iterators. Constant time operation.
  friend difference_type operator-(const Iterator &first,
                                 const Iterator &second) {
    return first._value - static_cast<difference_type>(second._value);
  }

  // Move the iterator forward by 1.
  Iterator &operator++() {
    return *this += 1;
  }

  // Create a new iterator 1 index greater than this.
  Iterator operator++(int) {
    Iterator current = *this;
    ++*this;
    return current;
  }

  // Move the iterator backward by 1.
  Iterator &operator--() {
    return *this += -1;
  }

  // Create a new iterator 1 index less than this.
  Iterator operator--(int) {
    Iterator current = *this;
    --*this;
    return current;
  }

  // Returns a const data reference.
  value_type operator[](difference_type index) const {
    if (index == 0) {
      return **this;
    }
    return *(*this + index);
  }

  // Returns a const data reference.
  value_type operator*() const {
    return _value;
  }

protected:
  Node<P, O> *_node;
  size_t _index;
  P _value;
};


} // namespace lava::data::slidx

#endif // LAVA_DATA_SLIDINGINDEX_ITERATOR_H_
