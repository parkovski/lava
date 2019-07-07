#ifndef ASH_COLLECTIONS_INTERVALTREE_ITERATORS_H_
#define ASH_COLLECTIONS_INTERVALTREE_ITERATORS_H_

#include "node.h"

#include <iterator>
#include <type_traits>

namespace ash::collections::itree {

// Yields U if T and U are the same ignoring const.
template<typename T, typename U>
struct if_noconst_same {
};

// Specialization for same T.
template<typename T>
struct if_noconst_same<T, T> {
  using type = T;
};

// Specialization for U = const T.
template<typename T>
struct if_noconst_same<T, const T> {
  using type = const T;
};

// Specialization for T = const U.
template<typename T>
struct if_noconst_same<const T, T> {
  using type = T;
};

// Yields U if T and U are the same ignoring const.
template<typename T, typename U>
using if_noconst_same_t = typename if_noconst_same<T, U>::type;

// Yields T if T is const.
template<typename T>
struct if_const {
};

// Specialization for const T.
template<typename T>
struct if_const<const T> {
  using type = const T;
};

// Yields T if T is const.
template<typename T>
using if_const_t = typename if_const<T>::type;

// Yields T if T is not const.
template<typename T>
struct if_mutable {
  using type = T;
};

// Empty specialization for const T.
template<typename T>
struct if_mutable<const T> {
};

// Yields T if T is not const.
template<typename T>
using if_mutable_t = typename if_mutable<T>::type;

// Yields mutable T when T is const, const T otherwise.
template<typename T>
struct opposite_const {
  using type = const T;
};

// Specialization to map const T -> T.
template<typename T>
struct opposite_const<const T> {
  using type = T;
};

// Yields mutable T when T is const, const T otherwise.
template<typename T>
using opposite_const_t = typename opposite_const<T>::type;

template<typename T>
class IntervalTree;

// Key type returned from iterators, allowing access to interval and data.
// This type contains a lot of const_casts and looks suspicious. Unfortunately,
// C++'s const only goes so far. This is essentially a proxy type for node
// with additional information (the exact position), and the constness of the
// inner data is conveyed through the type parameter. Basically, we have four
// different possibilities:
// - Key<T>: The key itself may be modified, as well as the data it refers to.
// - const Key<T>: The key may not be modified, but the data may.
// - Key<const T>: The key may be modified, but the data may not.
// - const Key<const T>: Neither the key nor the data may be modified.
// The reasoning behind this is that iterators need to hold the additional
// position information and be able to hand it out, but don't allow the
// position to be manipulated directly. Therefore, an iterator will return a
// const Key<T> &, and a const_iterator will return a const Key<const T> &.
// All the "weird stuff" here around const exists to enable this behavior.
template<typename T>
struct Key {
  friend struct Key<opposite_const_t<T>>;

  // Construct an invalid key.
  Key() = default;

  // Allow initialization from nullptr.
  Key(std::nullptr_t) noexcept
    : Key()
  {}

  // Construct a key with a node and absolute position for that node.
  Key(node_t<T> *node, size_t position) noexcept
    : _node(node), _position(position)
  {}

  Key(const Key &other) = default;

  Key &operator=(const Key &other) = default;

  // Assign from nullptr.
  Key &operator=(nullptr_t) noexcept {
    _node = nullptr;
    return *this;
  }

  // Equality comparison between two keys, regardless of constness of T.
  template<typename U, typename = if_noconst_same_t<T, U>>
  bool operator==(const Key<U> &other) {
    return _node == other._node;
  }

  // Inequality comparison between two keys, regardless of constness of T.
  template<typename U, typename = if_noconst_same_t<T, U>>
  bool operator!=(const Key<U> &other) {
    return !(*this == other);
  }

  // The key is true if the node is valid, otherwise it is false.
  explicit operator bool() const {
    return _node != nullptr;
  }

  // The start position (inclusive) of the interval.
  size_t begin() const {
    return _position;
  }

  // The end position (exclusive) of the interval.
  size_t end() const {
    return _position + _node->length();
  }

  // The length of the interval.
  size_t length() const {
    return _node->length();
  }

  // Determines of the node is on the parent's left.
  bool is_left() const {
    return _node->is_left();
  }

  // Determines of the node is on the parent's right.
  bool is_right() const {
    return _node->is_right();
  }

  // Gets the associated node.
  node_t<T> *node() {
    return _node;
  }

  // Gets the associated node.
  const node_t<T> *node() const {
    return _node;
  }

  // Data accessor. Returns a const reference when T is const, and a mutable
  // reference otherwise.
  T &operator*() const {
    return const_cast<Key *>(this)->_node->data();
  }

  // Data accessor. Returns a const pointer when T is const, and a mutable
  // reference otherwise.
  T *operator->() const {
    return &const_cast<Key *>(this)->_node->data();
  }

  // Conversion operator to allow using a const Key<T> & where a const
  // Key<const T> & is expected.
  template<typename = if_mutable_t<T>>
  operator const Key<const T> &() const {
    return *reinterpret_cast<const Key<const T> *>(this);
  }

  // Conversion from Key<T> & to Key<const T> &.
  template<typename = if_mutable_t<T>>
  operator Key<const T> &() {
    return *reinterpret_cast<Key<const T> *>(this);
  }

  // Gets the key for the parent of this node.
  Key parent() const {
    return {_node->parent(), _position - _node->offset()};
  }

  // Gets the key for the left child of this node.
  Key left() const {
    auto left = _node->left();
    if (!left) {
      return nullptr;
    }
    return {left, _position + left->offset()};
  }

  // Gets the key for the right child of this node.
  Key right() const {
    auto right = _node->right();
    if (!right) {
      return nullptr;
    }
    return {right, _position + right->offset()};
  }

private:
  node_t<T> *_node;
  size_t _position;
};

template<typename T>
class Iterator {
public:
  // Allow access to private members for conversion and comparison.
  friend class Iterator<opposite_const_t<T>>;

  // Allow the tree to access the node and position.
  friend class IntervalTree<T>;
  friend class IntervalTree<const T>;

  typedef void difference_type;
  typedef Key<T> value_type;
  typedef const Key<T> *pointer;
  typedef const Key<T> &reference;
  typedef std::bidirectional_iterator_tag iterator_category;

  // End (invalid) iterator constructor.
  explicit Iterator() = default;

  /// Iterator constructor for a node and absolute start position.
  /// \param node The initial node pointed to by this iterator.
  /// \param position The start position of \c node.
  explicit Iterator(node_t<T> *node, size_t position) noexcept
    : _key(node, position)
  {}

  // Copy constructor
  Iterator(const Iterator &other) = default;

  // Conversion from iterator to const_iterator.
  template<typename = if_const_t<T>>
  Iterator(const Iterator<std::remove_const_t<T>> &other) noexcept
    : _key(other._key)
  {}

  // Copy assignment
  Iterator &operator=(const Iterator &other) = default;

  // Conversion from iterator to const_iterator.
  template<typename = if_const_t<T>>
  Iterator &operator=(const Iterator<std::remove_const_t<T>> &other) noexcept {
    _key = other._key;
    return *this;
  }

  // Equality comparison against const or mutable iterators.
  template<typename U, typename = if_noconst_same_t<T, U>>
  bool operator==(const Iterator<U> &other) const {
    return _key == other._key;
  }

  // Inequality comparison against const or mutable iterators.
  template<typename U, typename = if_noconst_same_t<T, U>>
  bool operator!=(const Iterator<U> &other) const {
    return !(*this == other);
  }

  // Move to the next element.
  virtual Iterator &operator++() {
    if (!move_next()) {
      _key = nullptr;
    }
    return *this;
  }

  // Move to the next element, post-increment.
  Iterator operator++(int) {
    Iterator current = *this;
    ++*this;
    return current;
  }

  // Move to the previous element.
  virtual Iterator &operator--() {
    if (!move_prev()) {
      _key = nullptr;
    }
    return *this;
  }

  // Move to the previous element, post-decrement.
  Iterator operator--(int) {
    Iterator current = *this;
    --*this;
    return current;
  }

  const Key<T> &operator*() {
    return _key;
  }

  const Key<const T> &operator*() const {
    return _key;
  }

  const Key<T> *operator->() {
    return &_key;
  }

  const Key<const T> *operator->() const {
    return &_key;
  }

private:
  static bool no_condition(const Key<T> &) {
    return true;
  }

protected:
  template<
    typename F,
    typename = std::enable_if_t<
      std::is_invocable_r_v<bool, F, const Key<T> &>
    >
  >
  [[nodiscard]] bool move_up_if(const F &condition) {
    auto parent = _key.parent();
    if (!parent || !condition(parent)) {
      return false;
    }

    _key = parent;
    return true;
  }

  [[nodiscard]] bool move_up() {
    return move_up_if(Iterator::no_condition);
  }

  template<
    typename F,
    typename = std::enable_if_t<
      std::is_invocable_r_v<bool, F, const Key<T> &>
    >
  >
  [[nodiscard]] bool move_left_if(const F &condition) {
    auto left = _key.left();
    if (!left) {
      return false;
    }
    if (!condition(left)) {
      return false;
    }

    _key = left;
    return true;
  }

  [[nodiscard]] bool move_left() {
    return move_left_if(Iterator::no_condition);
  }

  template<
    typename F,
    typename = std::enable_if_t<
      std::is_invocable_r_v<bool, F, const Key<T> &>
    >
  >
  [[nodiscard]] bool move_right_if(const F &condition) {
    auto right = _key.right();
    if (!right) {
      return false;
    }
    if (!condition(right)) {
      return false;
    }

    _key = right;
    return true;
  }

  [[nodiscard]] bool move_right() {
    return move_right_if(Iterator::no_condition);
  }

  template<
    typename F,
    typename = std::enable_if_t<
      std::is_invocable_r_v<bool, F, const Key<T> &>
    >
  >
  [[nodiscard]] bool move_next_if(const F &condition) {
    auto key = _key;

    if (auto right = key.right(); right && condition(right)) {
      // If there is a right subtree, go all the way to its left side.
      key = right;
      auto left = key.left();
      while (left && condition(left)) {
        key = left;
        left = left.left();
      }
    } else {
      // No right subtree. To find the next node, go up until we are on
      // the left side of the parent. That parent node is the next node.
      while (key.is_right()) {
        key = key.parent();
      }
      auto parent = key.parent();
      if (!parent || !condition(parent)) {
        return false;
      }
      key = parent;
    }

    _key = key;
    return true;
  }

  [[nodiscard]] bool move_next() {
    return move_next_if(Iterator::no_condition);
  }

  template<
    typename F,
    typename = std::enable_if_t<
      std::is_invocable_r_v<bool, F, const Key<T> &>
    >
  >
  [[nodiscard]] bool move_prev_if(const F &condition) {
    auto key = _key;

    // Do the same as move_next_if, but in reverse.
    if (auto left = key.left(); left && condition(left)) {
      key = left;
      auto right = key.right();
      while (right && condition(right)) {
        key = right;
        right = right.right();
      }
    } else {
      while (key.is_left()) {
        key = key.parent();
      }
      auto parent = key.parent();
      if (!parent || !condition(parent)) {
        return false;
      }
      key = parent;
    }

    _key = key;
    return true;
  }

  [[nodiscard]] bool move_prev() {
    return move_prev_if(Iterator::no_condition);
  }

protected:
  // Key containing node and position.
  Key<T> _key;
};

} // namespace ash::collections::itree

#endif // ASH_COLLECTIONS_INTERVALTREE_ITERATORS_H_
