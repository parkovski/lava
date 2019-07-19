#ifndef ASH_COLLECTIONS_REDBLACKTREE_KEY_H_
#define ASH_COLLECTIONS_REDBLACKTREE_KEY_H_

#include "../detail.h"

namespace ash::collections::rbtree {

// Key type returned from iterators, allowing access to position and data.
// This type contains a lot of const_casts and looks suspicious. Unfortunately,
// C++'s const only goes so far. This is essentially a proxy type for node
// with additional information (the exact position), and the constness of the
// inner data is conveyed through the type parameter. Basically, we have four
// different possibilities:
// - Key<T>: The key itself may be modified, as well as the data it refers to.
// - const Key<T>: The key may not be modified, but the data may.
// - Key<const T>: The key may be modified, but the data may not.
// - const Key<const T>: Neither the key nor the data may be modified.
// In any case, although the node's data may be mutable, the node is not.
// The reasoning behind this is that iterators need to hold the additional
// position information and be able to hand it out, but don't allow the
// position to be manipulated directly. Therefore, an iterator will return a
// const Key<T> &, and a const_iterator will return a const Key<const T> &.
// All the "weird stuff" here around const exists to enable this behavior.
template<typename T, typename Traits>
struct Key {
  using node_type = typename Traits::template node_type<T>;
  using position_type = typename Traits::position_type;
  using offset_type = typename Traits::offset_type;

  friend struct Key<detail::opposite_const_t<T>, Traits>;

  // Construct an invalid key.
  Key() = default;

  // Allow initialization from nullptr.
  Key(std::nullptr_t) noexcept
    : Key()
  {}

  // Construct a key with a node and absolute position for that node.
  Key(node_type *node, position_type position) noexcept
    : _node(node), _position(position)
  {}

  Key(const Key &other) = default;

  Key &operator=(const Key &other) = default;

  // Assign from nullptr.
  Key &operator=(nullptr_t) noexcept {
    _node = nullptr;
    return *this;
  }

  bool operator==(const Key<const T, Traits> &other) const {
    return _node == other._node;
  }

  bool operator!=(const Key<const T, Traits> &other) const {
    return !(*this == other);
  }

  // The key is true if the node is valid, otherwise it is false.
  explicit operator bool() const {
    return _node != nullptr;
  }

  bool operator!() const {
    return _node == nullptr;
  }

  // The position of the node.
  position_type position() const {
    return _position;
  }

  // Determines if the node is on the parent's left.
  bool is_left() const {
    return _node->is_left();
  }

  // Determines if the node is on the parent's right.
  bool is_right() const {
    return _node->is_right();
  }

  // Gets the associated node.
  const node_type *node() const {
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
  template<typename = detail::if_mutable_t<T>>
  operator const Key<const T, Traits> &() const {
    return *reinterpret_cast<const Key<const T, Traits> *>(this);
  }

  // Conversion from Key<T> & to Key<const T> &.
  template<typename = detail::if_mutable_t<T>>
  operator Key<const T, Traits> &() {
    return *reinterpret_cast<Key<const T, Traits> *>(this);
  }

  // Gets the key for the parent of this node.
  Key parent() const {
    return {_node->parent(), _position - _node->offset()};
  }

  // Gets the key for the left child of this node.
  Key left() const {
    auto node = _node->left();
    if (!node) {
      return nullptr;
    }
    return {node, _position + node->offset()};
  }

  // Gets the key for the right child of this node.
  Key right() const {
    auto node = _node->right();
    if (!node) {
      return nullptr;
    }
    return {node, _position + node->offset()};
  }

private:
  node_type *_node;
  position_type _position;
};

} // namespace ash::collections::rbtree

#endif // ASH_COLLECTIONS_REDBLACKTREE_KEY_H_
