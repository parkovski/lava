#ifndef LAVA_DATA_SLIDINGINDEX_NODE_H_
#define LAVA_DATA_SLIDINGINDEX_NODE_H_

#include <type_traits>
#include <algorithm>

namespace lava::data::slidx {

// Type of the node colors, either red or black.
typedef bool color_t;

// Black nodes may appear anywhere, as long the number of black nodes from
// the root to any null leaf remains the same for any path.
static constexpr color_t Black = false;

// Red nodes may not have red children.
static constexpr color_t Red = true;

// Red-black tree node that stores subtree size and position offset from parent.
template<typename P, typename O>
struct Node {
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

  size_t size() const {
    return _size;
  }

  void set_size(size_t size) {
    _size = size;
  }

  // Should be called at the lowest modified level of the tree.
  // Recursively updates parent's size until reaching a point where the size
  // didn't change.
  void update_size() {
    size_t new_size = 1;
    if (_left) {
      new_size += _left->size();
    }
    if (_right) {
      new_size += _right->size();
    }
    if (new_size != _size) {
      _size = new_size;
      if (auto p = parent(); p) {
        p->update_size();
      }
    }
  }

private:
  /// Parent node or null if root. LSB contains the node color.
  Node *_parent = nullptr;
  /// Left child.
  Node *_left = nullptr;
  /// Right child.
  Node *_right = nullptr;
  /// Subtree size, including this node.
  size_t _size = 1;
  /// Interval start position offset from parent's start.
  offset_type _offset = 0;
};

} // namespace lava::data::slidx

#endif // LAVA_DATA_SLIDINGINDEX_NODE_H_
