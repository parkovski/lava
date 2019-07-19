#ifndef ASH_COLLECTIONS_REDBLACKTREE_ITERATOR_H_
#define ASH_COLLECTIONS_REDBLACKTREE_ITERATOR_H_

#include "key.h"
#include "node.h"

#include <iterator>
#include <type_traits>

namespace ash::collections::rbtree {

template<typename T, typename Traits>
class Iterator {
public:
  friend class Iterator<detail::opposite_const_t<T>, Traits>;

  typedef void difference_type;
  typedef Key<T, Traits> value_type;
  typedef const value_type *pointer;
  typedef const value_type &reference;
  typedef std::bidirectional_iterator_tag iterator_category;

  typedef Key<const T, Traits> const_value_type;
  typedef const const_value_type *const_pointer;
  typedef const const_value_type &const_reference;

  // End (invalid) iterator constructor.
  Iterator() = default;

  /// Iterator constructor for a node and absolute start position.
  /// \param key A key containing the starting node and position.
  explicit Iterator(reference key) noexcept
    : _key(key)
  {}

  Iterator(const Iterator &) = default;

  // Copy assignment
  Iterator &operator=(const Iterator &other) = default;

  // Conversion from iterator to const_iterator.
  template<typename = detail::if_const_t<T>>
  Iterator(const Iterator<std::remove_const_t<T>, Traits> &other) noexcept
    : _key(other._key)
  {}

  // Conversion from iterator to const_iterator.
  template<typename = detail::if_const_t<T>>
  Iterator &operator=(const Iterator<std::remove_const_t<T>, Traits> &other) noexcept {
    _key = other._key;
    return *this;
  }

  // Equality comparison against const or mutable iterators.
  template<typename U, typename = detail::if_noconst_same_t<T, U>>
  bool operator==(const Iterator<U, Traits> &other) const {
    return _key == other._key;
  }

  // Inequality comparison against const or mutable iterators.
  template<typename U, typename = detail::if_noconst_same_t<T, U>>
  bool operator!=(const Iterator<U, Traits> &other) const {
    return !(*this == other);
  }

  // Move to the next element.
  Iterator &operator++() {
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
  Iterator &operator--() {
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

  reference operator*() {
    return _key;
  }

  const_reference operator*() const {
    return _key;
  }

  pointer operator->() {
    return &operator*();
  }

  const_pointer operator->() const {
    return &operator*();
  }

private:
  static bool no_condition(reference) {
    return true;
  }

protected:
  template<
    typename F,
    typename = std::enable_if_t<
      std::is_invocable_r_v<bool, F, reference>
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
      std::is_invocable_r_v<bool, F, reference>
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
      std::is_invocable_r_v<bool, F, reference>
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
      std::is_invocable_r_v<bool, F, reference>
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
      std::is_invocable_r_v<bool, F, reference>
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
  value_type _key;
};

// Depth-first search iterator.
template<typename T, typename Traits>
class DFSIterator : public Iterator<T, Traits> {
public:
  friend class DFSIterator<detail::opposite_const_t<T>, Traits>;

  using Iterator::difference_type;
  using Iterator::value_type;
  using Iterator::pointer;
  using Iterator::reference;
  using Iterator::iterator_category;

  using Iterator::const_value_type;
  using Iterator::const_pointer;
  using Iterator::const_reference;

  using Iterator::Iterator;

  // Conversion from iterator to const_iterator.
  template<typename = detail::if_const_t<T>>
  DFSIterator(const DFSIterator<std::remove_const_t<T>, Traits> &other) noexcept
    : _key(other._key)
  {}

  // Conversion from iterator to const_iterator.
  template<typename = detail::if_const_t<T>>
  DFSIterator &operator=(const DFSIterator<std::remove_const_t<T>, Traits> &
                         other) noexcept {
    _key = other._key;
    return *this;
  }

  DFSIterator &operator++() {
    if (!move_left()) {
      while (!move_right()) {
        if (!move_up()) {
          _key = nullptr;
          return *this;
        }
      }
    }
    return *this;
  }

  DFSIterator operator++(int) {
    auto current = *this;
    ++*this;
    return current;
  }

  DFSIterator &operator--() {
    bool is_left = key.is_left();
    if (move_up()) {
      if (!is_left) {
        while (move_left()) {
        }
      }
    } else {
      _key = nullptr;
    }
    return *this;
  }

  DFSIterator operator--(int) {
    auto current = *this;
    --*this;
    return current;
  }
};

} // namespace ash::collections::rbtree

#endif // ASH_COLLECTIONS_REDBLACKTREE_ITERATOR_H_
