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

template<typename Reference, typename ConstReference>
Reference const_node_cast(ConstReference cr);

// Base red-black tree node component for child/parent pointers and color.
template<typename Self>
struct NodeBase {
  /// Construct a new empty node.
  explicit NodeBase() noexcept
  {}

  // Nodes cannot be moved or copied, so it is ok to hold pointers to their
  // inner data.

  NodeBase(const NodeBase &) = delete;
  NodeBase &operator=(const NodeBase &) = delete;
  NodeBase(NodeBase &&) = delete;
  NodeBase &operator=(NodeBase &&) = delete;

  // Recursively delete children.
  ~NodeBase() {
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

  Self *parent() {
    return reinterpret_cast<Self *>(reinterpret_cast<size_t>(_parent)
                                    & static_cast<size_t>(-2));
    return _parent;
  }

  const Self *parent() const {
    return const_cast<Self *>(this)->parent();
  }

  Self *set_parent(Self *new_parent) {
    size_t color_mask = reinterpret_cast<size_t>(_parent) & 1;
    _parent = reinterpret_cast<Self *>(reinterpret_cast<size_t>(new_parent)
                                       | color_mask);
    return new_parent;
  }

  Self *sibling() {
    if (!parent()) {
      return nullptr;
    }
    if (parent()->left() == this) {
      return parent()->right();
    }
    return parent()->left();
  }

  const Self *sibling() const {
    return const_cast<Self *>(this)->sibling();
  }

  Self *left() {
    return _left;
  }

  const Self *left() const {
    return _left;
  }

  bool is_left() const {
    return parent() && this == parent()->left();
  }

  Self *set_left(Self *new_left) {
    if (new_left) {
      new_left->set_parent(this);
    }
    return _left = new_left;
  }

  Self *right() {
    return _right;
  }

  const Self *right() const {
    return _right;
  }

  bool is_right() const {
    return parent() && this == parent()->right();
  }

  Self *set_right(Self *new_right) {
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
      _parent = reinterpret_cast<Self *>(reinterpret_cast<size_t>(_parent)
                                         | 1);
    } else {
      _parent = reinterpret_cast<Self *>(reinterpret_cast<size_t>(_parent)
                                         & static_cast<size_t>(-2));
    }
  }

private:
  /// Parent node or null if root. LSB contains the node color.
  Self *_parent = nullptr;
  /// Left child.
  Self *_left = nullptr;
  /// Right child.
  Self *_right = nullptr;
};

// Standard component for a node key.
template<typename K>
struct NodeKey {
  template<typename = std::enable_if_t<std::is_default_constructible_v<K>>>
  explicit NodeKey()
    noexcept(std::is_nothrow_default_constructible_v<K>)
    : _key()
  {}

  explicit NodeKey(K key)
    noexcept(noexcept(K(std::move_if_noexcept(key))))
    : _key(std::move_if_noexcept(key))
  {}

  K &key() {
    return _key;
  }

  const K &key() const {
    return _key;
  }

private:
  K _key;
};

// Component for an offset-based node key. Offsets are relative to the parent,
// which allows efficient moving of spans of nodes.
template<typename K, typename O>
struct NodeKeyOffset {
  template<typename = std::enable_if_t<std::is_default_constructible_v<O>>>
  explicit NodeKeyOffset()
    noexcept(std::is_nothrow_default_constructible_v<O>)
    : _offset()
  {}

  explicit NodeKeyOffset(O offset)
    noexcept(noexcept(O(std::move_if_noexcept(offset))))
    : _offset(std::move_if_noexcept(offset))
  {}

  O &offset() {
    return _offset;
  }

  const O &offset() const {
    return _offset;
  }

  void set_offset(O &&offset) {
    _offset = std::move(offset);
  }

  void set_offset(const O &offset) {
    _offset = offset;
  }

  K key(const K &parent_key) const {
    return parent_key + _offset;
  }

  void set_key(const K &key, const K &parent_key) {
    _offset = key - parent_key;
  }

private:
  O _offset;
};

// Node value component which supports valueless (void) nodes.
template<typename V, bool = std::is_void_v<V>>
struct NodeValue {
  template<typename = std::enable_if_t<std::is_default_constructible_v<V>>>
  NodeValue()
    noexcept(std::is_nothrow_default_constructible_v<V>)
    : _value()
  {}

  /// Construct a new node with only the data field filled in.
  /// \param args The arguments to pass to the data constructor.
  template<typename... Args>
  explicit NodeValue(Args &&...args)
    noexcept(std::is_nothrow_constructible_v<V, Args...>)
    : _value(std::forward<Args>(args)...)
  {}

  const V &value() const & {
    return _value;
  }

  V &value() & {
    return _value;
  }

  V &&value() && {
    return std::move(_value);
  }

private:
  V _value;
};

// Empty value for void.
template<typename V>
struct NodeValue<V, true> {};

template<typename N>
struct NodePtr;

template<typename N>
struct NodeBaseRef;

// Const node reference proxy wrapper.
template<typename N>
struct NodeBaseRef<const N> {
  friend struct NodePtr<const N>;
  friend struct NodePtr<N>;

  NodeBaseRef(const N &node) noexcept
    : _node(const_cast<N *>(&node))
  {}

  NodePtr<const N> parent() const
  { return NodeRef(*_node->parent()); }

  NodePtr<const N> sibling() const
  { return _node->sibling(); }

  NodePtr<const N> left() const
  { return _node->left(); }

  bool is_left() const
  { return _node->is_left(); }

  NodePtr<const N> right() const
  { return _node->right(); }

  bool is_right() const
  { return _node->is_right(); }

  color_t color() const
  { return _node->color(); }

  operator const N &() const {
    return *_node;
  }

protected:
  mutable N *_node;
};

/// Reference proxy type for \c NodeBase.
template<typename N>
struct NodeBaseRef : virtual NodeBaseRef<const N> {
  NodeBaseRef(N &node) noexcept
    : NodeBaseRef<const N>(node)
  {}

  void unlink() const
  { this->_node->unlink(); }

  NodePtr<N> parent() const
  { return this->_node->parent(); }

  NodePtr<N> set_parent(NodePtr<N> new_parent) const
  { return this->_node->set_parent(new_parent); }

  NodePtr<N> sibling() const
  { return this->_node->sibling(); }

  NodePtr<N> left() const
  { return this->_node->left(); }

  NodePtr<N> set_left(NodePtr<N> new_left) const
  { return this->_node->set_left(new_left); }

  NodePtr<N> right() const
  { return this->_node->right(); }

  NodePtr<N> set_right(NodePtr<N> new_right) const
  { return this->_node->set_right(new_right); }

  void set_color(color_t new_color) const
  { this->_node->set_color(new_color); }

  operator N &() const {
    return *this->_node;
  }
};

template<typename N, typename T = std::remove_cv_t<typename N::value_type>>
struct NodeValueRef;

// Valueless const proxy wrapper.
template<typename N>
struct NodeValueRef<const N, void> : virtual NodeBaseRef<const N> {};

// Valueless proxy wrapper.
template<typename N>
struct NodeValueRef<N, void> : virtual NodeValueRef<const N>,
                               virtual NodeBaseRef<N> {};

// Const value proxy wrapper.
template<typename N, typename T>
struct NodeValueRef<const N, T> : virtual NodeBaseRef<const N> {
  const typename N::value_type &value() const {
    return this->_node->value();
  }
};

// Node and value proxy wrapper.
template<typename N, typename T>
struct NodeValueRef : virtual NodeValueRef<const N>,
                            virtual NodeBaseRef<N> {
  typename N::value_type &value() const & {
    return this->_node->value();
  }

  typename N::value_type &&value() && {
    return std::move(*this->_node).value();
  }
};

template<typename N>
struct NodePtr<const N> {
  explicit NodePtr(const N *node) noexcept
    : _ref(*const_cast<N *>(node))
  {}

  NodePtr(const std::remove_reference_t<typename N::const_reference> *ref)
    noexcept
    : _ref(const_node_cast<typename N::reference>(*ref))
  {}

  typename N::const_reference &operator*() {
    return _ref;
  }

  const typename N::const_reference &operator*() const {
    return _ref;
  }

  typename N::const_reference *operator->() {
    return &_ref;
  }

  const typename N::const_reference *operator->() const {
    return &_ref;
  }

  bool operator==(const N *other) const {
    return _ref._node == other;
  }

  bool operator!=(const N *other) const {
    return !(*this == other);
  }

  friend bool operator==(const N *node, const NodePtr<const N> &self) {
    return self == node;
  }

  friend bool operator!=(const N *node, const NodePtr<const N> &self) {
    return self != node;
  }

  explicit operator bool() const {
    return _ref._node != nullptr;
  }

  bool operator!() const {
    return _ref._node == nullptr;
  }

  operator const N *() const {
    return _ref._node;
  }

  NodePtr<N> const_node_cast() const {
    return NodePtr<N>(std::addressof(_ref));
  }

protected:
  mutable typename N::reference _ref;
};

/// Pointer type for \c NodeRef proxy.
template<typename N>
struct NodePtr : NodePtr<const N> {
  explicit NodePtr(N *node) noexcept
    : NodePtr<const N>(node)
  {}

  NodePtr(const std::remove_reference_t<typename N::reference> *ref) noexcept
    : NodePtr<const N>(ref)
  {}

  typename N::reference &operator*() {
    return this->_ref;
  }

  const typename N::reference &operator*() const {
    return this->_ref;
  }

  typename N::reference *operator->() {
    return &this->_ref;
  }

  const typename N::reference *operator->() const {
    return &this->_ref;
  }

  operator N *() const {
    return this->_ref._node;
  }
};

template<typename K, typename V>
struct Node : NodeBase<Node<K, V>>, NodeKey<K>, NodeValue<V> {
  using key_type = K;
  using value_type = V;

  using pointer = Node *;
  using reference = Node &;
  using const_reference = const Node &;

  template<typename = std::enable_if_t<std::is_default_constructible_v<K> &&
                                        std::is_default_constructible_v<
                                          NodeValue<V>
                                        >
                                      >>
  Node()
    noexcept(std::is_nothrow_default_constructible_v<K> &&
             std::is_nothrow_default_constructible_v<NodeValue<V>>)
  {}

  template<typename... Args>
  explicit Node(K key, Args &&...args)
    noexcept(std::is_nothrow_constructible_v<V, Args...> &&
             std::is_nothrow_move_constructible_v<NodeKey<K>>)
    : NodeBase(),
      NodeKey(std::move_if_noexcept(key)),
      NodeValue(std::forward<Args>(args)...)
  {}
};

template<typename K, typename O, typename V>
struct OffsetNode : NodeBase<OffsetNode<K, O, V>>, NodeKeyOffset<K, O>,
                    NodeValue<V> {
  using key_type = K;
  using value_type = V;

  using pointer = NodePtr<OffsetNode>;

  template<typename = std::enable_if_t<std::is_default_constructible_v<O> &&
                                        std::is_default_constructible_v<
                                          NodeValue<V>
                                        >
                                      >
  OffsetNode()
    noexcept(std::is_nothrow_default_constructible_v<O> &&
             std::is_nothrow_default_constructible_v<NodeValue<V>>)
  {}

  template<typename... Args>
  explicit OffsetNode(O offset, Args &&...args)
    noexcept(std::is_nothrow_constructible_v<V, Args...> &&
             std::is_nothrow_constructible_v<NodeKeyOffset<O>, O>)
    : NodeBase(),
      NodeKeyOffset(std::move_if_noexcept(offset)),
      NodeValue(std::forward<Args>(args)...)
  {}

  struct const_reference : virtual NodeValueRef<const OffsetNode> {
    explicit const_reference(const OffsetNode &node)
      noexcept(noexcept(K(std::declval<const O &>())))
      : NodeBaseRef<const OffsetNode>(node),
        _key(node.offset())
    {}

    const_reference(const OffsetNode &node, const K &key)
      noexcept(std::is_nothrow_copy_constructible_v<K>)
      : NodeBaseRef<const OffsetNode>(node),
        _key(key)
    {}

    NodePtr<const OffsetNode> left() const {
      auto left = this->_node->left();
      if (!left) {
        return nullptr;
      }
      return {left, _key + left->offset()};
    }

    NodePtr<const OffsetNode> right() const {
      auto right = this->_node->right();
      if (!right) {
        return nullptr;
      }
      return {right, _key + right->offset()};
    }

    NodePtr<const OffsetNode> parent() const {
      auto parent = this->_node->parent();
      if (!parent) {
        return nullptr;
      }
      return {parent, _key - _node->offset()};
    }

    NodePtr<const OffsetNode> sibling() const {
      auto sibling = this->_node->sibling();
      if (!sibling) {
        return nullptr;
      }
      return {sibling, _key - _node->offset() + sibling->offset()};
    }

    const K &key() const {
      return _key;
    }

    const O &offset() const {
      return this->_node->offset();
    }

    NodePtr<const OffsetNode> operator&() const {
      return NodePtr<const OffsetNode>(this);
    }

    struct reference as_mutable_reference() const;

  protected:
    mutable K _key;
  };

  struct reference : virtual NodeValueRef<OffsetNode>, virtual const_reference {
    explicit reference(OffsetNode &node)
      noexcept(noexcept(K(std::declval<const O &>())))
      : const_reference(node)
    {}

    reference(OffsetNode &node, const K &key)
      noexcept(std::is_nothrow_copy_constructible_v<K>)
      : const_reference(node, key)
    {}

    void set_key(const K &new_key) const {
      this->_node->set_offset(new_key - _key + this->_node->offset());
      _key = new_key;
    }

    void set_key(K &&new_key) const {
      this->_node->set_offset(new_key - _key + this->_node->offset());
      _key = std::move(new_key);
    }

    void set_offset(const O &offset) const {
      this->_node->set_offset(offset);
    }

    void set_offset(O &&offset) const {
      this->_node->set_offset(std::move(offset));
    }

    NodePtr<OffsetNode> set_left(NodePtr<OffsetNode> new_left) const {
      if (new_left) {
        new_left->set_parent(*this);
      }
      this->_node->set_left(new_left);
      return new_left;
    }

    NodePtr<OffsetNode> set_right(NodePtr<OffsetNode> new_right) const {
      if (new_right) {
        new_right->set_parent(*this);
      }
      this->_node->set_right(new_right);
      return new_right;
    }

    NodePtr<OffsetNode> set_parent(NodePtr<OffsetNode> new_parent) const {
      if (new_parent) {
        this->_node->set_key(_key, new_parent->key());
      } else {
        this->_node->set_offset(_key);
      }
      this->_node->set_parent(new_parent);
      return new_parent;
    }

    NodePtr<OffsetNode> operator&() const {
      return NodePtr<OffsetNode>(this);
    }
  };

  reference const_reference::as_mutable_reference() const {
    return reference{*this->_node, this->_key};
  }
};

template<typename Node>
Node &const_node_cast<Node &, const Node &>(const Node &cr) {
  return const_cast<Node &>(cr);
}

template<typename Node, template<typename> typename Reference>
Reference<Node>
const_node_cast<Reference<Node>, Reference<const Node>>(
  Reference<const Node> cr
) {
  return cr.as_mutable_reference();
}

} // namespace ash::collections::rbtree

#endif // ASH_COLLECTIONS_REDBLACKTREE_NODE_H_
