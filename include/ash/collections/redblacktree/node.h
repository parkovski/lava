#ifndef ASH_COLLECTIONS_REDBLACKTREE_NODE_H_
#define ASH_COLLECTIONS_REDBLACKTREE_NODE_H_

#include <type_traits>
#include <algorithm>

namespace ash::collections::rbtree {

// Type of the node colors, either red or black.
typedef bool color_t;

// Black nodes may appear anywhere, as long the number of black nodes from
// the root to any null leaf remains the same for any path.
static constexpr color_t Black = false;

// Red nodes may not have red children.
static constexpr color_t Red = true;

template<typename T, typename P, typename O>
struct Node;

// Base red-black tree node with no data. Note: the destructor is not virtual!
template<typename P, typename O>
struct Node<void, P, O> {
  using position_type = P;
  using offset_type = O;

  /// Construct a new empty node.
  explicit Node() noexcept
  {}

  // Nodes cannot be moved or copied, so it is ok to hold pointers to their
  // inner data.

  Node(const Node &) = delete;
  Node &operator=(const Node &) = delete;
  Node(Node &&) = delete;
  Node &operator=(Node &&) = delete;

  // Recursively delete children.
  ~Node() {
    if (left()) {
      delete left();
    }
    if (right()) {
      delete right();
    }
  }

  // Removes references to this node's parent, left and right, but does _not_
  // remove their references to this.
  void unlink() {
    set_parent(nullptr);
    set_left(nullptr);
    set_right(nullptr);
  }

  Node *parent() {
    return reinterpret_cast<Node *>(reinterpret_cast<size_t>(_parent)
                                    & static_cast<size_t>(-2));
    return _parent;
  }

  const Node *parent() const {
    return const_cast<Node *>(this)->parent();
  }

  Node *set_parent(Node *new_parent) {
    size_t color_mask = reinterpret_cast<size_t>(_parent) & 1;
    _parent = reinterpret_cast<Node *>(reinterpret_cast<size_t>(new_parent)
                                       | color_mask);
    return new_parent;
  }

  Node *sibling() {
    if (!parent()) {
      return nullptr;
    }
    if (parent()->left() == this) {
      return parent()->right();
    }
    return parent()->left();
  }

  const Node *sibling() const {
    return const_cast<Node *>(this)->sibling();
  }

  Node *left() {
    return _left;
  }

  const Node *left() const {
    return _left;
  }

  bool is_left() const {
    return parent() && this == parent()->left();
  }

  Node *set_left(Node *new_left) {
    if (new_left) {
      new_left->set_parent(this);
    }
    return _left = new_left;
  }

  Node *right() {
    return _right;
  }

  const Node *right() const {
    return _right;
  }

  bool is_right() const {
    return parent() && this == parent()->right();
  }

  Node *set_right(Node *new_right) {
    if (new_right) {
      new_right->set_parent(this);
    }
    return _right = new_right;
  }

  color_t color() const {
    return reinterpret_cast<size_t>(_parent) & 1;
  }

  void set_color(color_t new_color) {
    if (new_color) {
      _parent = reinterpret_cast<Node *>(reinterpret_cast<size_t>(_parent)
                                         | 1);
    } else {
      _parent = reinterpret_cast<Node *>(reinterpret_cast<size_t>(_parent)
                                         & static_cast<size_t>(-2));
    }
  }

  offset_type offset() const {
    return _offset;
  }

  void set_offset(offset_type new_offset) {
    _offset = new_offset;
  }

  position_type parent_position(position_type position) const {
    return position - _offset;
  }

  position_type position(position_type parent_pos) const {
    return parent_pos + _offset;
  }

  void set_position(position_type parent_pos, position_type new_pos) {
    _offset = static_cast<offset_type>(new_pos) - parent_pos;
  }

private:
  /// Parent node or null if root. LSB contains the node color.
  Node *_parent = nullptr;
  /// Left child.
  Node *_left = nullptr;
  /// Right child.
  Node *_right = nullptr;
  /// Interval start position offset from parent's start.
  offset_type _offset = 0;
};

template<typename T, typename P, typename O>
struct Node : Node<void, P, O> {
  using position_type = P;
  using offset_type = O;

  /// Construct a new node with only the data field filled in.
  /// \param args The arguments to pass to the data constructor.
  template<typename... Args>
  explicit Node(Args &&...args)
    noexcept(std::is_nothrow_constructible_v<T, Args...>)
    : _data(std::forward<Args>(args)...)
  {}

  Node *parent() {
    return static_cast<Node *>(Node<void, P, O>::parent());
  }

  const Node *parent() const {
    return static_cast<const Node *>(Node<void, P, O>::parent());
  }

  Node *set_parent(Node *new_parent) {
    return static_cast<Node *>(Node<void, P, O>::set_parent(new_parent));
  }

  Node *sibling() {
    return static_cast<Node *>(Node<void, P, O>::sibling());
  }

  const Node *sibling() const {
    return static_cast<const Node *>(Node<void, P, O>::sibling());
  }

  Node *left() {
    return static_cast<Node *>(Node<void, P, O>::left());
  }

  const Node *left() const {
    return static_cast<const Node *>(Node<void, P, O>::left());
  }

  Node *set_left(Node *new_left) {
    return static_cast<Node *>(Node<void, P, O>::set_left(new_left));
  }

  Node *right() {
    return static_cast<Node *>(Node<void, P, O>::right());
  }

  const Node *right() const {
    return static_cast<const Node *>(Node<void, P, O>::right());
  }

  Node *set_right(Node *new_right) {
    return static_cast<Node *>(Node<void, P, O>::set_right(new_right));
  }

  const T &data() const & {
    return _data;
  }

  T &data() & {
    return _data;
  }

  T &&data() && {
    return std::move(_data);
  }

  template<typename = std::enable_if_t<std::is_move_assignable_v<T>>>
  void set_data(T &&new_data)
    noexcept(std::is_nothrow_move_assignable_v<T>)
  {
    _data = std::move(new_data);
  }

  template<typename = std::enable_if_t<std::is_copy_assignable_v<T>>>
  void set_data(const T &new_data)
    noexcept(std::is_nothrow_copy_assignable_v<T>)
  {
    _data = new_data;
  }

private:
  T _data;
};

}

#endif // ASH_COLLECTIONS_REDBLACKTREE_NODE_H_
