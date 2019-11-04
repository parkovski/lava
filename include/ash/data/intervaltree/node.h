#ifndef ASH_DATA_INTERVALTREE_NODE_H_
#define ASH_DATA_INTERVALTREE_NODE_H_

#include <type_traits>
#include <algorithm>
#include <cstddef>

namespace ash::data::itree {

// Type of the node colors, either red or black.
typedef bool color_t;

// Black nodes may appear anywhere, as long the number of black nodes from
// the root to any null leaf remains the same for any path.
static constexpr color_t Black = false;

// Red nodes may not have red children.
static constexpr color_t Red = true;

// Red-black interval tree node.
template<typename T>
struct Node {
  /// Construct a new node with only the data field filled in.
  /// \param args The arguments to pass to the data constructor.
  template<typename... Args>
  explicit Node(Args &&...args)
    noexcept(std::is_nothrow_constructible_v<Args...>)
    : _data(std::forward<Args>(args)...)
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

  // Update this node's subtree length by comparing its length and its
  // children's subtree lengths.
  bool update_max() {
    ptrdiff_t left_max;
    ptrdiff_t right_max;
    const ptrdiff_t my_max = length();

    if (left()) {
      left_max = left()->max_offset() + left()->offset();
    } else {
      left_max = 0;
    }

    if (right()) {
      right_max = right()->max_offset() + right()->offset();
    } else {
      right_max = 0;
    }

    auto max = std::max({my_max, left_max, right_max});

    if (max != _max_offset) {
      set_max_offset(max);
      return true;
    }

    return false;
  }

  // Keeps updating the subtree lengths of the parent until one of them
  // contains a length that is not dependent on the updated node's length.
  template<bool Inserting>
  void update_max_recursive() {
    bool updated = update_max();
    auto parent = this->parent();
    if (!updated || !parent) {
      return;
    }

    // When inserting, the ranges can only grow. When deleting, they can only
    // shrink.
    if constexpr (Inserting) {
      if (_max_offset - _offset > parent->_max_offset) {
        parent->template update_max_recursive<true>();
      }
    } else {
      if (_max_offset - _offset < parent->_max_offset) {
        parent->template update_max_recursive<false>();
      }
    }
  }

  Node *parent() const {
    return reinterpret_cast<Node *>(reinterpret_cast<size_t>(_parent)
                                    & static_cast<size_t>(-2));
  }

  Node *set_parent(Node *new_parent) {
    size_t color_mask = reinterpret_cast<size_t>(_parent) & 1;
    _parent = reinterpret_cast<Node *>(reinterpret_cast<size_t>(new_parent)
                                       | color_mask);
    return new_parent;
  }

  Node *sibling() const {
    if (!parent()) {
      return nullptr;
    }
    if (parent()->left() == this) {
      return parent()->right();
    }
    return parent()->left();
  }

  Node *left() const {
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

  Node *right() const {
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

  size_t length() const {
    return _length;
  }

  void set_length(size_t new_length) {
    _length = new_length;
  }

  color_t color() const {
    return reinterpret_cast<size_t>(_parent) & 1;
  }

  void set_color(color_t new_color) {
    if (new_color == Red) {
      _parent = reinterpret_cast<Node *>(reinterpret_cast<size_t>(_parent)
                                         | 1);
    } else {
      _parent = reinterpret_cast<Node *>(reinterpret_cast<size_t>(_parent)
                                         & static_cast<size_t>(-2));
    }
  }

  ptrdiff_t offset() const {
    return _offset;
  }

  void set_offset(ptrdiff_t new_offset) {
    _offset = new_offset;
  }

  ptrdiff_t max_offset() const {
    return _max_offset;
  }

  void set_max_offset(ptrdiff_t max_offset) {
    _max_offset = max_offset;
  }

  size_t parent_position(size_t position) const {
    return position - _offset;
  }

  size_t position(size_t parent_pos) const {
    return parent_pos + _offset;
  }

  void set_position(size_t parent_pos, size_t new_pos) {
    _offset = static_cast<ptrdiff_t>(new_pos) - parent_pos;
  }

  size_t max_pos(size_t position) const {
    return position + _max_offset;
  }

  void set_max_pos(size_t position, size_t max_position) {
    _max_offset = max_position - position;
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
  /// Parent node or null if root. LSB contains node color.
  Node *_parent = nullptr;
  /// Left child.
  Node *_left = nullptr;
  /// Right child.
  Node *_right = nullptr;
  /// Interval start position offset from parent's start.
  ptrdiff_t _offset = 0;
  /// Maximum offset of subtree relative to my position.
  ptrdiff_t _max_offset = 0;
  /// Interval length and node color.
  size_t _length = 0;
  /// Data
  T _data;
};

// Resolves node_t<T> to Node<T> and node_t<const T> to const Node<T>.
template<typename T>
struct node_type {
  using type = Node<T>;
};

// Specialization for const T -> const Node<T>.
template<typename T>
struct node_type<const T> {
  using type = const typename node_type<T>::type;
};

// Resolves node_t<T> to Node<T> and node_t<const T> to const Node<T>.
template<typename T>
using node_t = typename node_type<T>::type;

} // namespace ash::data::itree

#endif // ASH_DATA_INTERVALTREE_NODE_H_
