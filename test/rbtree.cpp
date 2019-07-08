#include <iostream>
#include <sstream>
#include <cmath>
#include <type_traits>
#include <algorithm>
#include <functional>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min
#undef max

// Utility {{{

// Adds const to a type if IsConst is true.
template<typename T, bool IsConst>
struct maybe_const;

template<typename T>
struct maybe_const<T, true> {
  using type = const T;
};

template<typename T>
struct maybe_const<T, false> {
  using type = T;
};

// }}} Utility

// Node {{{

template<typename T>
class Tree;

// Red-black interval tree node.
template<typename T>
struct Node {
  using Tree = Tree<T>;

  // Type of the node colors, either red or black.
  typedef bool color_t;

  // Black nodes may appear anywhere, as long the number of black nodes from
  // the root to any null leaf remains the same for any path.
  static constexpr color_t Black = false;

  // Red nodes may not have red children.
  static constexpr color_t Red = true;

  /// Construct a new node with empty children.
  Node(Node *parent, ptrdiff_t offset, size_t length, T &&data)
    noexcept(std::is_nothrow_move_constructible_v<T>)
    : _parent(parent),
      _left(nullptr),
      _right(nullptr),
      _offset(offset),
      _length(length),
      _min_offset(0),
      _max_offset(length),
      _data(std::move(data))
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
  bool update_min_max() {
    ptrdiff_t left_min, left_max;
    ptrdiff_t right_min, right_max;
    const ptrdiff_t my_min = 0;
    const ptrdiff_t my_max = length();

    if (left()) {
      left_min = left()->min_offset() + left()->offset();
      left_max = left()->max_offset() + left()->offset();
    } else {
      left_min = left_max = 0;
    }

    if (right()) {
      right_min = right()->min_offset() + right()->offset();
      right_max = right()->max_offset() + right()->offset();
    } else {
      right_min = right_max = 0;
    }

    auto min = std::min({my_min, left_min, right_min});
    auto max = std::max({my_max, left_max, right_max});

    if (min != _min_offset || max != _max_offset) {
      set_min_max(min, max);
      return true;
    }

    return false;
  }

  // Keeps updating the subtree lengths of the parent until one of them
  // contains a length that is not dependent on the updated node's length.
  template<bool Inserting>
  void update_min_max_recursive() {
    bool updated = update_min_max();
    auto parent = this->parent();
    if (!updated || !parent) {
      return;
    }

    // When inserting, the ranges can only grow. When deleting, they can only
    // shrink.
    if constexpr (Inserting) {
      if (_max_offset - _offset > parent->_max_offset ||
          _min_offset - _offset < parent->_min_offset) {
        parent->update_min_max_recursive<true>();
      }
    } else {
      if (_max_offset - _offset < parent->_max_offset ||
          _min_offset - _offset > parent->_min_offset) {
        parent->update_min_max_recursive<false>();
      }
    }
  }

  Node *parent() const {
    return _parent;
  }

  Node *set_parent(Node *new_parent) {
    return _parent = new_parent;
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

  Node *set_left(Node *new_left) {
    if (new_left) {
      new_left->set_parent(this);
    }
    return _left = new_left;
  }

  Node *right() const {
    return _right;
  }

  Node *set_right(Node *new_right) {
    if (new_right) {
      new_right->set_parent(this);
    }
    return _right = new_right;
  }

  size_t length() const {
    return _length & ~ColorBit;
  }

  void set_length(size_t new_length) {
    _length = (_length & ColorBit) | (new_length & ~ColorBit);
  }

  color_t color() const {
    return (_length & ColorBit) == ColorBit;
  }

  void set_color(color_t new_color) {
    if (new_color == Red) {
      _length |= ColorBit;
    } else {
      _length &= ~ColorBit;
    }
  }

  ptrdiff_t offset() const {
    return _offset;
  }

  void set_offset(ptrdiff_t new_offset) {
    _offset = new_offset;
  }

  ptrdiff_t min_offset() const {
    return _min_offset;
  }

  ptrdiff_t max_offset() const {
    return _max_offset;
  }

  void set_min_max(ptrdiff_t min_offset, ptrdiff_t max_offset) {
    _min_offset = min_offset;
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

  size_t min_pos(size_t position) const {
    return position + _min_offset;
  }

  size_t max_pos(size_t position) const {
    return position + _max_offset;
  }

  void set_min_max_pos(size_t position, size_t min_position,
                       size_t max_position)
  {
    _min_offset = min_position - position;
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

  void set_data(T &&new_data) {
    _data = std::move(new_data);
  }

  void set_data(const T &new_data) {
    _data = new_data;
  }

  unsigned print(size_t parent_pos, unsigned cursor) {
    const size_t pos = _offset + parent_pos;

    const size_t min = min_pos(pos),
                 max = max_pos(pos);

    const unsigned textlen
      = unsigned(1 + log10(pos))
      + unsigned(1 + log10(length()))
      + unsigned(1 + log10(pos + length()))
      + unsigned(1 + log10(min))
      + unsigned(1 + log10(max))
      + 20; // parens and labels
    
    std::cout << "\n\n";

    unsigned leftlen, rightlen;
    if (_left) {
      leftlen = _left->print(pos, cursor);
    } else {
      std::cout << "\x1b[" << (cursor + 1) << "Gnull   ";
      leftlen = 4;
    }

    const auto selfmid = textlen / 2;
    if (selfmid > leftlen) {
      leftlen = selfmid;
    }

    std::cout << "\x1bM\x1b[" << (cursor + leftlen + 1) << "G/ \\"
              << "\x1bM\x1b[" << (cursor + leftlen - selfmid + 1) << "G";
    if (color() == Red) {
      std::cout << "\x1b[31m";
    }
    std::cout << "(S=" << pos << ", E=" << pos + length() << ", L=" << length()
              << ", m=" << min << ", M=" << max << ")\n\n";
    if (color() == Red) {
      std::cout << "\x1b[m";
    }

    if (_right) {
      rightlen = _right->print(pos, cursor + leftlen + 4);
    } else {
      std::cout << "\x1b[" << (cursor + leftlen + 4) << "Gnull";
      rightlen = 4;
    }

    std::cout << "\x1b[2A";

    return leftlen + rightlen + 6;
  }

private:
  /// This bit in length stores the node color.
  /// FIXME
  static constexpr size_t ColorBit = 0xf0000000;
  /// Parent node or null if root.
  Node *_parent;
  /// Left child.
  Node *_left;
  /// Right child.
  Node *_right;
  /// Interval start position offset from parent's start.
  ptrdiff_t _offset;
  /// Minimum offset of subtree relative to my position.
  ptrdiff_t _min_offset;
  /// Maximum offset of subtree relative to my position.
  ptrdiff_t _max_offset;
  /// Interval length and node color.
  size_t _length;
  /// Associated data.
  T _data;
};

// }}} Node

// Red-black interval tree. Ordered first by length, then from top to bottom
// by total subtree minimum and maximum position.
template<typename T>
class Tree {
public:
  using Node = Node<T>;
  using color_t = typename Node::color_t;
  static constexpr color_t Red = Node::Red;
  static constexpr color_t Black = Node::Black;

  // Standard in-order iterator.
  template<bool IsConst>
  class Iterator {
  protected:
    // Node type with appropriate const-ness.
    using node_type = typename maybe_const<Node, IsConst>::type *;

    // Allow const iterator to access members for converting constructor and
    // assignment operator.
    friend class Iterator<true>;

    // Allow the tree to access the node and position.
    friend class Tree;

    // Current node pointed to by this iterator.
    node_type _node;

    // Position of the current node.
    size_t _position;

  public:
    /// \param node The initial node pointed to by this iterator.
    /// \param position The position of \c node.
    explicit Iterator(node_type node, size_t position) noexcept
      : _node(node),
        _position(position)
    {}

    // End iterator constructor.
    explicit Iterator() noexcept
      : _node(nullptr),
        _position(0)
    {}

  private:
    static bool no_condition(const Node *, size_t) {
      return true;
    }

  protected:
    template<
      typename F,
      typename = std::enable_if_t<
        std::is_invocable_r_v<bool, F, const Node *, size_t>
      >
    >
    [[nodiscard]] bool move_up_if(const F &condition) {
      auto parent = _node->parent();
      auto parent_position = _node->parent_position(_position);
      if (!parent || !condition(parent, parent_position)) {
        return false;
      }

      _position = parent_position;
      _node = parent;
      return true;
    }

    [[nodiscard]] bool move_up() {
      return move_up_if(Iterator::no_condition);
    }

    template<
      typename F,
      typename = std::enable_if_t<
        std::is_invocable_r_v<bool, F, const Node *, size_t>
      >
    >
    [[nodiscard]] bool move_left_if(const F &condition) {
      auto left = _node->left();
      if (!left) {
        return false;
      }
      auto left_position = left->position(_position);
      if (!condition(left, left_position)) {
        return false;
      }

      _node = left;
      _position = left_position;
      return true;
    }

    [[nodiscard]] bool move_left() {
      return move_left_if(Iterator::no_condition);
    }

    template<
      typename F,
      typename = std::enable_if_t<
        std::is_invocable_r_v<bool, F, const Node *, size_t>
      >
    >
    [[nodiscard]] bool move_right_if(const F &condition) {
      auto right = _node->right();
      if (!right) {
        return false;
      }
      auto right_position = right->position(_position);
      if (!condition(right, right_position)) {
        return false;
      }

      _node = right;
      _position = right_position;
      return true;
    }

    [[nodiscard]] bool move_right() {
      return move_right_if(Iterator::no_condition);
    }

    template<
      typename F,
      typename = std::enable_if_t<
        std::is_invocable_r_v<bool, F, const Node *, size_t>
      >
    >
    [[nodiscard]] bool move_next_if(const F &condition) {
      auto node = _node;
      auto position = _position;

      if (node->right() &&
          condition(node->right(), node->right()->position(position))) {
        // If there is a right subtree, go all the way to its left side.
        node = node->right();
        position += node->offset();
        while (node->left() &&
               condition(node->left(), node->left()->position(position))) {
          node = node->left();
          position += node->offset();
        }
      } else {
        // No right subtree. To find the next node, go up until we are on
        // the left side of the parent. That parent node is the next node.
        while (node->parent() && node == node->parent()->right()) {
          position -= node->offset();
          node = node->parent();
        }
        if (!node->parent() ||
            !condition(node->parent(), node->parent_position(position))) {
          return false;
        }
        position -= node->offset();
        node = node->parent();
      }

      _position = position;
      _node = node;
      return true;
    }

    [[nodiscard]] bool move_next() {
      return move_next_if(Iterator::no_condition);
    }

    template<
      typename F,
      typename = std::enable_if_t<
        std::is_invocable_r_v<bool, F, const Node *, size_t>
      >
    >
    [[nodiscard]] bool move_prev_if(const F &condition) {
      auto node = _node;
      auto position = _position;

      // Do the same as move_next_if, but in reverse.
      if (node->left() &&
          condition(node->left(), node->left()->position(position))) {
        node = node->left();
        position += node->offset();
        while (node->right() &&
               condition(node->right(), node->right()->position(position))) {
          node = node->right();
          position += node->offset();
        }
      } else {
        while (node->parent() && node == node->parent()->left()) {
          position -= node->offset();
          node = node->parent();
        }
        if (!node->parent() ||
            !condition(node->parent(), node->parent_position(position))) {
          return false;
        }
        position -= node->offset();
        node = node->parent();
      }

      _position = position;
      _node = node;
      return true;
    }

    [[nodiscard]] bool move_prev() {
      return move_prev_if(Iterator::no_condition);
    }

  public:
    typedef void difference_type;
    typedef typename maybe_const<T, IsConst>::type value_type;
    typedef typename maybe_const<T, IsConst>::type *pointer;
    typedef typename maybe_const<T, IsConst>::type &reference;
    typedef std::bidirectional_iterator_tag iterator_category;

    Iterator(const Iterator &other) = default;

    // Conversion from iterator to const_iterator.
    template<typename = std::enable_if_t<IsConst>>
    Iterator(const Iterator<false> &it) noexcept
      : _node(it._node),
        _position(it._position)
    {}

    Iterator &operator=(const Iterator &other) = default;

    // Conversion from iterator to const_iterator.
    template<typename R = std::enable_if_t<IsConst, Iterator &>>
    R operator=(const Iterator<false> &other) noexcept {
      _node = other._node;
      _position = other._position;
      return *this;
    }

    bool operator==(const Iterator &other) const {
      return _node == other._node;
    }

    bool operator!=(const Iterator &other) const {
      return !(*this == other);
    }

    template<typename R = std::enable_if_t<!IsConst, T &>>
    R operator*() {
      return _node->data();
    }

    const T &operator*() const {
      return _node->data();
    }

    template<typename R = std::enable_if_t<!IsConst, T *>>
    R operator->() {
      return &_node->data();
    }

    const T *operator->() const {
      return &_node->data();
    }

    ptrdiff_t position() const {
      return _position;
    }

    size_t length() const {
      return _node->length();
    }

    virtual Iterator &operator++() {
      if (!move_next()) {
        _node = nullptr;
      }
      return *this;
    }

    Iterator operator++(int) {
      Iterator current = *this;
      ++*this;
      return current;
    }

    virtual Iterator &operator--() {
      if (!move_prev()) {
        _node = nullptr;
      }
      return *this;
    }

    Iterator operator--(int) {
      Iterator current = *this;
      --*this;
      return current;
    }
  };

  template<bool IsConst>
  class BaseSearchIterator : public Iterator<IsConst> {
  protected:
    using typename Iterator<IsConst>::node_type;

    friend class BaseSearchIterator<true>;

    size_t _start;
    size_t _end;

    explicit BaseSearchIterator(node_type node, size_t position,
                                size_t start, size_t end) noexcept
      : Iterator<IsConst>(node, position),
        _start(start),
        _end(end)
    {}

  public:
    using typename Iterator<IsConst>::difference_type;
    using typename Iterator<IsConst>::value_type;
    using typename Iterator<IsConst>::pointer;
    using typename Iterator<IsConst>::reference;
    using typename Iterator<IsConst>::iterator_category;

    BaseSearchIterator(const BaseSearchIterator &other) = default;

    template<typename = std::enable_if_t<IsConst>>
    BaseSearchIterator(const BaseSearchIterator<false> &it) noexcept
      : Iterator<true>(it),
        _start(it._start),
        _end(it._end)
    {}

    BaseSearchIterator &operator=(const BaseSearchIterator &other) = default;

    template<typename R = std::enable_if_t<IsConst, BaseSearchIterator &>>
    R operator=(const BaseSearchIterator<false> &other) noexcept {
      Iterator<true>::operator=(other);
      _start = other._start;
      _end = other._end;
      return *this;
    }

    bool operator==(const BaseSearchIterator &other) const {
      return _node == other._node;
    }

    bool operator!=(const BaseSearchIterator &other) const {
      return !(*this == other);
    }

    bool operator==(const Iterator<IsConst> &other) const {
      return _node == nullptr && other == Iterator<IsConst>();
    }

    bool operator!=(const Iterator<IsConst> &other) const {
      return !(*this == other);
    }
  };

# define IMPLEMENT_SEARCH_ITERATOR(Class)                                      \
    using typename Iterator<IsConst>::node_type;                               \
                                                                               \
  public:                                                                      \
    using typename BaseSearchIterator<IsConst>::difference_type;               \
    using typename BaseSearchIterator<IsConst>::value_type;                    \
    using typename BaseSearchIterator<IsConst>::pointer;                       \
    using typename BaseSearchIterator<IsConst>::reference;                     \
    using typename BaseSearchIterator<IsConst>::iterator_category;             \
                                                                               \
    explicit Class(node_type node, size_t position, size_t start,              \
                   size_t end) noexcept                                        \
      : BaseSearchIterator<IsConst>(node, position, start, end)                \
    {                                                                          \
      if (node) {                                                              \
        find_first();                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    /* Copy constructor */                                                     \
    Class(const Class &other) noexcept                                         \
      : BaseSearchIterator<IsConst>(other)                                     \
    {}                                                                         \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename = std::enable_if_t<IsConst>>                             \
    Class(const Class<false> &other) noexcept                                  \
      : BaseSearchIterator<true>(other)                                        \
    {                                                                          \
    }                                                                          \
                                                                               \
    /* Copy assignment */                                                      \
    Class &operator=(const Class &other) noexcept {                            \
      BaseSearchIterator::operator=(other);                                    \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename R = std::enable_if_t<IsConst, Class &>>                  \
    R operator=(const Class<false> &other) noexcept {                          \
      BaseSearchIterator::operator=(other);                                    \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    /* All search iterators may be constructed or assigned from another. */    \
    Class(const BaseSearchIterator<IsConst> &other) noexcept                   \
      : BaseSearchIterator<IsConst>(other)                                     \
    {                                                                          \
      if (!is_match()) {                                                       \
        find_first();                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename = std::enable_if_t<IsConst>>                             \
    Class(const BaseSearchIterator<false> &other) noexcept                     \
      : BaseSearchIterator<true>(other)                                        \
    {                                                                          \
      if (!is_match()) {                                                       \
        find_first();                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    /* All search iterators may be constructed or assigned from another. */    \
    Class &operator=(const BaseSearchIterator<IsConst> &other) noexcept {      \
      BaseSearchIterator<IsConst>::operator=(other);                           \
      if (!is_match()) {                                                       \
        find_first();                                                          \
      }                                                                        \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename R = std::enable_if_t<IsConst, Class &>>                  \
    R operator=(const BaseSearchIterator<false> &other) noexcept {             \
      BaseSearchIterator<true>::operator=(other);                              \
      if (!is_match()) {                                                       \
        find_first();                                                          \
      }                                                                        \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    bool operator==(const Class &other) const {                                \
      return _node == other._node;                                             \
    }                                                                          \
                                                                               \
    bool operator!=(const Class &other) const {                                \
      return !(*this == other);                                                \
    }                                                                          \
                                                                               \
    /* Only for comparison against end. */                                     \
    bool operator==(const Iterator<IsConst> &other) const {                    \
      return BaseSearchIterator<IsConst>::operator==(other);                   \
    }                                                                          \
                                                                               \
    /* Only for comparison against end. */                                     \
    bool operator!=(const Iterator<IsConst> &other) const {                    \
      return BaseSearchIterator<IsConst>::operator!=(other);                   \
    }                                                                          \
                                                                               \
    Class &operator++() override {                                             \
      auto const is_possible_search_node =                                     \
        [this](const Node * node, size_t pos) {                                \
          return this->is_possible_search_node(node, pos);                     \
        };                                                                     \
                                                                               \
      while (move_next_if(is_possible_search_node)) {                          \
        if (is_match()) {                                                      \
          return *this;                                                        \
        }                                                                      \
      }                                                                        \
                                                                               \
      /* Nothing found. */                                                     \
      _node = nullptr;                                                         \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    Class operator++(int) {                                                    \
      Class current = *this;                                                   \
      ++*this;                                                                 \
      return current;                                                          \
    }                                                                          \
                                                                               \
    Class &operator--() override {                                             \
      auto const is_possible_search_node =                                     \
        [this](const Node * node, size_t pos) {                                \
          return this->is_possible_search_node(node, pos);                     \
        };                                                                     \
                                                                               \
      while (move_prev_if(is_possible_search_node)) {                          \
        if (is_match()) {                                                      \
          return *this;                                                        \
        }                                                                      \
      }                                                                        \
                                                                               \
      _node = nullptr;                                                         \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    Class operator--(int) {                                                    \
      Class current = *this;                                                   \
      --*this;                                                                 \
      return current;                                                          \
    }                                                                          \
                                                                               \
  private:                                                                     \

  // This iterator finds ranges that are fully outside the search range. That
  // is, all returned ranges fully overlap the search range.
  template<bool IsConst>
  class OuterSearchIterator final : public BaseSearchIterator<IsConst> {
    IMPLEMENT_SEARCH_ITERATOR(OuterSearchIterator)

    // For an outer range search, we only care about the subtree with lengths
    // of at least the search length, where the min is at or before the start
    // and the max is at or after the end.
    bool is_possible_search_node(const Node *node, size_t pos) const {
      const size_t search_length = _end - _start;

      return node->length() >= search_length &&
             node->min_pos(pos) <= _start &&
             node->max_pos(pos) >= _end;
    }

    // Checks whether the current node fully covers the search range.
    bool is_match() const {
      return _position <= _start &&
             _position + _node->length() >= _end;
    }

    // Find the shortest (left-most) interval that satisfies the search.
    void find_first() {
      auto const search_length = _end - _start;

      // First find the subtree with lengths greater or equal to the search
      // length.
      while (_node->length() < search_length) {
        if (!move_right()) {
          // No match found.
          _node = nullptr;
          return;
        }
      }

      using namespace std::placeholders;
      auto const is_possible_search_node = std::bind(
        &OuterSearchIterator::is_possible_search_node,
        this, _1, _2
      );

      // Look down the left side until max - min is too small.
      while (move_left_if(is_possible_search_node)) {
      }

      // Is it the minimum possible?
      if (is_match()) {
        return;
      }

      // Not found yet. Is it to the right?
      while (move_next_if(is_possible_search_node)) {
        if (is_match()) {
          return;
        }
      }

      // Didn't find anything.
      _node = nullptr;
    }
  };

  // This iterator finds ranges that are completely inside the search range.
  template<bool IsConst>
  class InnerSearchIterator final : public BaseSearchIterator<IsConst> {
    IMPLEMENT_SEARCH_ITERATOR(InnerSearchIterator)

    // Subtrees of interest have a length no greater than the search length,
    // a min before the end of the search range, and a max before its start.
    bool is_possible_search_node(const Node *node, size_t pos) {
      if (node->length() > _end - _start) {
        return false;
      }

      return node->min_pos(pos) < _end &&
             node->max_pos(pos) > _start;
    }

    // A match starts at or after the search start and ends at or before the
    // search end.
    bool is_match() const {
      return _position >= _start &&
             _position + _node->length() <= _end;
    }

    void find_first() {
      auto const search_length = _end - _start;

      // First find the subtree with lengths less or equal to the search length.
      while (true) {
        auto const node_length = _node->length();
        if (node_length <= search_length) {
          break;
        }

        if (!move_left()) {
          // No match found.
          _node = nullptr;
          return;
        }
      }

      using namespace std::placeholders;
      auto const is_possible_search_node = std::bind(
        &InnerSearchIterator::is_possible_search_node,
        this, _1, _2
      );

      // Look down the left side until min is too high or max is too low.
      while (move_left_if(is_possible_search_node)) {
      }

      // This would be the minimum match.
      if (is_match()) {
        return;
      }

      // Otherwise look at greater nodes.
      while (move_next_if(is_possible_search_node)) {
        if (is_match()) {
          return;
        }
      }

      // Didn't find anything.
      _node = nullptr;
    }
  };

  // This iterator finds ranges that overlap the search range at some point.
  template<bool IsConst>
  class OverlapSearchIterator final : public BaseSearchIterator<IsConst> {
    IMPLEMENT_SEARCH_ITERATOR(OverlapSearchIterator)

    // For overlapping nodes, length doesn't matter and we have to look at
    // any node with a min before end and a max after start.
    bool is_possible_search_node(const Node *node, size_t pos) const {
      return node->min_pos(pos) < _end && node->max_pos(pos) > _start;
    }

    // A match is any node that starts before the end of the search range and
    // ends after its start.
    bool is_match() const {
      return _position < _end && _position + _node->length() > _start;
    }

    void find_first() {
      using namespace std::placeholders;
      auto const is_possible_search_node = std::bind(
        &OverlapSearchIterator::is_possible_search_node,
        this, _1, _2
      );

      // We have to search everything that meets is_possible_search_node.
      // Just start from the left, and if we don't find anything start looking
      // at the right subtree.
      while (move_left_if(is_possible_search_node)) {
      }

      // If it's here, it's the left-most one.
      if (is_match()) {
        return;
      }

      // Otherwise, start looking right.
      while (move_next_if(is_possible_search_node)) {
        if (is_match()) {
          return;
        }
      }

      // Nothing matching was found.
      _node = nullptr;
    }
  };

  // This iterator finds ranges that are equal to the search range.
  template<bool IsConst>
  class EqualSearchIterator final : public BaseSearchIterator<IsConst> {
    IMPLEMENT_SEARCH_ITERATOR(EqualSearchIterator)

    // Rule out any nodes whose length is not equal to the target length
    // or whose min and max are out of range.
    bool is_possible_search_node(const Node *node, size_t pos) const {
      if (node->length() != _end - _start) {
        return false;
      }
      if (node->min_pos(pos) > _start || node->max_pos(pos) < _end) {
        return false;
      }

      return true;
    }

    // A match has to start and end exactly on the search range.
    bool is_match() const {
      return _position == _start &&
             _position + _node->length() == _end;
    }

    void find_first() {
      const size_t search_length = _end - _start;

      // First find the subtree with matching length, if it exists, making
      // sure to only look at subtrees with a valid min and max.
      while (_node->min_pos(_position) <= _start
          && _node->max_pos(_position) >= _end) {
        auto const node_length = _node->length();
        if (node_length > search_length) {
          if (!move_left()) {
            // No matching length exists.
            _node = nullptr;
            return;
          }
        } else if (node_length < search_length) {
          if (!move_right()) {
            // No matching length exists.
            _node = nullptr;
            return;
          }
        } else {
          // Lengths are equal.
          break;
        }
      }

      // Find the first element with a matching start position.
      using namespace std::placeholders;
      auto const is_possible_search_node = std::bind(
        &EqualSearchIterator::is_possible_search_node,
        this, _1, _2
      );

      // First move as far left as possible.
      while (move_left_if(is_possible_search_node)) {
      }

      // If we found it, this is the left-most one.
      if (is_match()) {
        return;
      }

      // If it wasn't found, look to the right.
      while (move_next_if(is_possible_search_node)) {
        if (is_match()) {
          return;
        }
      }

      // No match found.
      _node = nullptr;
    }
  };

# undef IMPLEMENT_SEARCH_ITERATOR

  using const_iterator = Iterator<true>;
  using iterator = Iterator<false>;

  using const_base_search_iterator = BaseSearchIterator<true>;
  using base_search_iterator = BaseSearchIterator<false>;

  using const_outer_search_iterator = OuterSearchIterator<true>;
  using outer_search_iterator = OuterSearchIterator<false>;

  using const_inner_search_iterator = InnerSearchIterator<true>;
  using inner_search_iterator = InnerSearchIterator<false>;

  using const_overlap_search_iterator = OverlapSearchIterator<true>;
  using overlap_search_iterator = OverlapSearchIterator<false>;

  using const_equal_search_iterator = EqualSearchIterator<true>;
  using equal_search_iterator = EqualSearchIterator<false>;

  explicit Tree() noexcept
    : _root(nullptr)
  {}

  Tree(Tree &&other) noexcept {
    *this = std::move(other);
  }

  Tree &operator=(Tree &&other) noexcept {
    _root = other._root;
    other._root = nullptr;
  }

#if 0
  Tree(const Tree &other) {
    _root = new Node(*other._root);
  }

  Tree &operator=(const Tree &other) {
    _root = new Node(*other._root);
  }
#endif
  // Returns an iterator over all elements in the tree, starting with the
  // shortest.
  iterator begin() {
    return iterator(_root, _root->offset());
  }

  // Returns a const_iterator over all elements in the tree, starting with the
  // shortest.
  const_iterator begin() const {
    return const_iterator(_root, _root->offset());
  }

  // Returns a const_iterator over all elements in the tree, starting with the
  // shortest.
  const_iterator cbegin() const {
    return begin();
  }

  // Returns a past-the-end iterator.
  iterator end() {
    return iterator();
  }

  // Returns a past-the-end const_iterator.
  const_iterator end() const {
    return const_iterator();
  }

  // Returns a past-the-end const_iterator.
  const_iterator cend() const {
    return end();
  }

  // Returns an iterator over all nodes that entirely overlap the range
  // [start, end).
  outer_search_iterator find_outer(size_t start, size_t end) {
    return outer_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns a const_iterator over all nodes that entirely overlap the range
  // [start, end).
  const_outer_search_iterator find_outer(size_t start, size_t end) const {
    return const_outer_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns an iterator over all nodes inside the range [start, end).
  inner_search_iterator find_inner(size_t start, size_t end) {
    return inner_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns a const_iterator over all nodes inside the range [start, end).
  const_inner_search_iterator find_inner(size_t start, size_t end) const {
    return const_inner_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns an iterator over nodes that overlap [start, end) at any point.
  overlap_search_iterator find_overlap(size_t start, size_t end) {
    return overlap_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns a const_iterator over nodes that overlap [start, end) at any
  // point.
  const_overlap_search_iterator find_overlap(size_t start, size_t end) const {
    return const_overlap_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns an iterator over all nodes with the same range as the search
  // range.
  equal_search_iterator find_equal(size_t start, size_t end) {
    return equal_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns a const_iterator over all nodes with the same range as the search
  // range.
  const_equal_search_iterator find_equal(size_t start, size_t end) const {
    return const_equal_search_iterator(_root, _root->offset(), start, end);
  }

  // Adds an interval to the tree.
  T &insert(size_t position, size_t length, T &&data) {
    ptrdiff_t offset = static_cast<ptrdiff_t>(position);
    if (!_root) {
      _root = new Node(nullptr, offset, length, std::move(data));
      return _root->data();
    }

    // Find the appropriate leaf for the new node.
    Node *node = _root;
    while (true) {
      offset -= node->offset();

      if (length < node->length()) {
        if (node->left()) {
          node = node->left();
          continue;
        }
        node = node->set_left(new Node(node, offset, length,
                                       std::move(data)));
        break;
      } else {
        if (node->right()) {
          node = node->right();
          continue;
        }
        node = node->set_right(new Node(node, offset, length,
                                        std::move(data)));
        break;
      }
    }

    // Fix interval tracking.
    node->parent()->update_min_max_recursive<true>();
    // Fix red-black properties.
    fix_for_insert(node);

    return node->data();
  }

  // Requires where != end.
  void erase(const_iterator where) {
    // We have a mutable reference to the tree, which has a mutable reference
    // to this node somewhere, we just don't want to go digging around trying
    // to find it.
    auto node = const_cast<Node *>(where._node);
    auto position = where._position;
    auto parent = node->parent();
    auto color = node->color();
    Node *child;

    // To delete a node, we need to "move" it into a position where it has
    // no children. For a node with two children, we first move its successor
    // into its place, where the target node now has zero or one child.
    // For a node with one child, we then just move the child into its place.
    // Now the target node does not have any children and can be deleted.

    if (node->right() && node->left()) {
      // 2 children. Any node with a right child has a next node that is all
      // the way down the left subtree of that child - its left is empty.
      // Move the successor into the original node's place, and then move
      // the successor's child into the successor's place.
      ++where;
      auto next_node = const_cast<Node *>(where._node);
      auto next_position = where._position;
      child = next_node->right();

      if (child) {
        child->set_offset(child->offset() + next_node->offset());
      }

      auto next_parent = next_node->parent();
      if (parent) {
        if (parent->left() == node) {
          parent->set_left(next_node);
        } else {
          parent->set_right(next_node);
        }
        next_node->set_position(node->parent_position(position), next_position);
      } else {
        next_node->set_parent(nullptr);
        next_node->set_position(0, next_position);
        _root = next_node;
      }

      // It's possible that next_parent == node, so it's important that these
      // links get set before the ones on next_node below.
      if (next_node == next_parent->left()) {
        next_parent->set_left(child);
        parent = next_parent;
      } else {
        // If it's to the right, it has to be directly right of node, therefore
        // next_parent == node. The child, currently node's right, will be
        // moved under next_node.
        next_parent->set_right(child);
        parent = next_node;
      }

      next_node->set_color(color);

      auto left = node->left();
      next_node->set_left(left);
      if (left) {
        left->set_position(next_position, left->position(position));
        left->update_min_max();
      }

      auto right = node->right();
      next_node->set_right(right);
      if (right) {
        right->set_position(next_position, right->position(position));
        right->update_min_max();
      }

      // Because the removed node was above parent here, its subtree cannot
      // have gotten any smaller, but may have gotten larger. Nodes above
      // next_node may have smaller subtrees, however, but these recursive
      // calls will not overlap.
      parent->update_min_max_recursive<true>();
      next_node->update_min_max_recursive<false>();
    } else {
      // Zero or one child. Just move the child up to node's position.
      if (!(child = node->right())) {
        child = node->left();
      }
      // Move child into the place of node.
      if (parent) {
        if (node == parent->left()) {
          parent->set_left(child);
        } else {
          parent->set_right(child);
        }
        if (child) {
          child->set_offset(child->offset() + node->offset());
        }
        // Because a node was removed, the subtree length may now be smaller.
        parent->update_min_max_recursive<false>();
      } else {
        _root = child;
        if (child) {
          child->set_parent(nullptr);
          child->set_offset(child->offset() + node->offset());
          child->update_min_max();
        }
      }
    }

    // Now node has zero or one child (child may be null). Replace it with
    // its child and fix the tree to maintain red-black properties.
    if (child && color == Black) {
      if (child->color() == Red) {
        child->set_color(Black);
      } else if (parent) {
        fix_for_erase(child);
      }
    }
    node->unlink();
    delete node;
  }

  void print() {
    if (!_root) {
      return;
    }
    _root->print(0, 0);
    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
  }

private:
  // When inserting a new node, fixes the tree to maintain red-black
  // properties.
  void fix_for_insert(Node *node) {
    auto parent = node->parent();

    if (!parent) {
      // Root must be black.
      node->set_color(Black);
      return;
    }

    // Assume we're inserting a red node to start with, so not to add a black
    // node to only one path.
    node->set_color(Red);

    if (parent->color() == Black) {
      // Red child of black parent: ok.
      return;
    }

    // We know there's a grandparent because parent is red, which the root
    // cannot be.
    auto grandparent = parent->parent();
    bool parent_is_left = parent == grandparent->left();
    auto uncle = parent_is_left ? grandparent->right() : grandparent->left();
    if (uncle && uncle->color() == Red) {
      // Parent, uncle, and node are red, therefore grandparent is black.
      // When a node becomes black, a higher up node must become red to
      // maintain the black node count property. But since grandparent was
      // black, its parent could have been red, which case we will have to
      // recursively fix that.
      parent->set_color(Black);
      uncle->set_color(Black);
      grandparent->set_color(Red);
      fix_for_insert(grandparent);
      return;
    }

    // Parent and node are red, but uncle and grandparent are black.
    // If node is on the inside (from grandparent, left then right or right
    // then left), we have to first rotate it to the outside of its parent so
    // that grandparent to child is a "straight line".
    // Then we can rotate around the grandparent and swap the new parent and
    // grandparent's colors to get rid of the double red.
    if (node == parent->right() && parent_is_left) {
      rotate_left(parent);
      fix_for_insert_rotate(parent);
    } else if (node == parent->left() && !parent_is_left) {
      rotate_right(parent);
      fix_for_insert_rotate(parent);
    } else {
      fix_for_insert_rotate(node);
    }
  }

  // Do the final rotation to get rid of two red nodes in a row on the same
  // side (left->left or right->right).
  void fix_for_insert_rotate(Node *node) {
    auto parent = node->parent();
    auto grandparent = parent->parent();

    if (node == parent->left()) {
      rotate_right(grandparent);
    } else {
      rotate_left(grandparent);
    }

    // At this point, grandparent is at the same height as node, and they are
    // both children of parent. Parent and node are red, grandparent is
    // black.  Swap parent and grandparent's colors to satisfy the no double
    // red property.
    parent->set_color(Black);
    grandparent->set_color(Red);
  }

  // Assumes node is black, parent is valid.
  void fix_for_erase(Node *node) {
    auto parent = node->parent();
    auto sibling = node->sibling();

    if (sibling->color() == Red) {
      parent->set_color(Red);
      sibling->set_color(Black);
      if (node == parent->left()) {
        rotate_left(parent);
      } else {
        rotate_right(parent);
      }
      parent = node->parent();
      sibling = node->sibling();
    }

    if (parent->color() == Black &&
        sibling->color() == Black &&
        (!sibling->left() || sibling->left()->color() == Black) &&
        (!sibling->right() || sibling->right()->color() == Black)) {
      sibling->set_color(Red);
      fix_for_erase(parent);
      return;
    }

    if (parent->color() == Red &&
        sibling->color() == Black &&
        (!sibling->left() || sibling->left()->color() == Black) &&
        (!sibling->right() || sibling->right()->color() == Black)) {
      sibling->set_color(Red);
      parent->set_color(Black);
      return;
    }

    if (sibling->color() == Black) {
      if (node == parent->left() &&
          (!sibling->right() || sibling->right()->color() == Black) &&
          (sibling->left() && sibling->left()->color() == Red)) {
        sibling->set_color(Red);
        sibling->left()->set_color(Black);
        rotate_right(sibling);
      } else if (node == parent->right() &&
                 (!sibling->left() || sibling->left()->color() == Black) &&
                 (sibling->right() && sibling->right()->color() == Red)) {
        sibling->set_color(Red);
        sibling->right()->set_color(Black);
        rotate_left(sibling);
      }
      parent = node->parent();
      sibling = node->sibling();
    }

    sibling->set_color(parent->color());
    parent->set_color(Black);
    if (node == parent->left()) {
      if (sibling->right()) {
        sibling->right()->set_color(Black);
      }
      rotate_left(parent);
    } else {
      if (sibling->left()) {
        sibling->left()->set_color(Black);
      }
      rotate_right(parent);
    }
  }

  void fix_for_rotate(Node *old_pivot, Node *new_pivot, Node *parent) {
    auto old_pivot_offset = old_pivot->offset();
    auto new_pivot_offset = new_pivot->offset();

    old_pivot->set_offset(-new_pivot_offset);
    new_pivot->set_offset(old_pivot_offset + new_pivot_offset);

    new_pivot->set_min_max(old_pivot->min_offset() - new_pivot_offset,
                           old_pivot->max_offset() - new_pivot_offset);
    old_pivot->update_min_max();

    if (old_pivot == _root) {
      (_root = new_pivot)->set_parent(nullptr);
    } else if (old_pivot == parent->left()) {
      parent->set_left(new_pivot);
    } else {
      parent->set_right(new_pivot);
    }
  }

  void rotate_left(Node *pivot) {
    auto new_pivot = pivot->right();
    auto parent = pivot->parent();
    pivot->set_right(new_pivot->left());
    new_pivot->set_left(pivot);

    fix_for_rotate(pivot, new_pivot, parent);
  }

  void rotate_right(Node *pivot) {
    auto new_pivot = pivot->left();
    auto parent = pivot->parent();
    pivot->set_left(new_pivot->right());
    new_pivot->set_right(pivot);

    fix_for_rotate(pivot, new_pivot, parent);
  }

  Node *_root;
};

int main() {
  Tree<int> tree;

  auto hstdout = GetStdHandle(STD_OUTPUT_HANDLE);
  DWORD outmode;
  GetConsoleMode(hstdout, &outmode);
  SetConsoleMode(hstdout, ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING | DISABLE_NEWLINE_AUTO_RETURN);

  tree.insert(1, 5, 1050);
  tree.insert(2, 3, 2030);
  tree.insert(4, 7, 4070);
  tree.insert(3, 9, 3090);
  tree.insert(2, 4, 2040);
  tree.insert(1, 9, 1090);
  tree.insert(4, 5, 4050);
  tree.insert(2, 6, 2060);
  tree.insert(8, 9, 8090);
  tree.insert(5, 8, 5080);
  tree.insert(5, 9, 5090);
  tree.insert(1, 2, 1020);

  tree.print();

  std::string inp;
  std::cout << "Search modes: [ou]tside, [i]nside, [ov]erlap, [e]qual.\n";
  std::cout << "Also: [a]dd, [d]elete, [p]rint\n";

  while (true) {
    int mode = 0;
    std::cout << "Mode > ";
    std::getline(std::cin, inp);
    if (inp.length()) {
      if (inp[0] == 'o' && inp.length() > 1) {
        if (inp[1] == 'u') {
          mode = 1;
        } else if (inp[1] == 'v') {
          mode = 3;
        }
      } else if (inp[0] == 'i') {
        mode = 2;
      } else if (inp[0] == 'e') {
        mode = 4;
      } else if (inp[0] == 'a') {
        mode = 5;
      } else if (inp[0] == 'd') {
        mode = 6;
      } else if (inp[0] == 'p') {
        mode = 7;
      }
    }
    if (mode == 0) {
      break;
    }
    if (mode == 7) {
      tree.print();
      continue;
    }
    std::cout << "Start> ";
    std::getline(std::cin, inp);
    int start = atoi(inp.c_str());
    std::cout << "End  > ";
    std::getline(std::cin, inp);
    int end = atoi(inp.c_str());

    //auto printmatches = [&tree](auto it) {
    auto printmatches = [&tree](Tree<int>::base_search_iterator &it) {
      auto end = tree.end();
      while (it != end) {
        std::cout << "Found (" << it.position() << ", "
                  << it.position() + it.length() << ")\n";
        ++it;
      }
    };

    switch (mode) {
    case 1:
      printmatches(tree.find_outer(start, end));
      break;
    case 2:
      printmatches(tree.find_inner(start, end));
      break;
    case 3:
      printmatches(tree.find_overlap(start, end));
      break;
    case 4:
      printmatches(tree.find_equal(start, end));
      break;
    case 5:
      tree.insert(start, end, 0);
      std::cout << "Inserted.\n";
      break;
    case 6:
      {
        auto it = tree.find_equal(start, end);
        if (it == tree.end()) {
          std::cout << "Not found!\n";
          break;
        }
        tree.erase(it);
        std::cout << "Erased.\n";
      }
      break;
    // case 7: print. see above.
    default:
      std::cout << "Did you forget to add a mode?\n";
      break;
    }
  }

  SetConsoleMode(hstdout, outmode);

  return 0;
}
