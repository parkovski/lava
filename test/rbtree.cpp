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

template<typename T, bool = std::is_const_v<T>>
struct maybe_const {
  using const_type = const T;
};

template<typename T>
struct maybe_const<T, true> {
  using type = const T;

  template<typename U>
  using when_const_t = U;
};

template<typename T>
struct maybe_const<T, false> {
  using type = T;
  using mutable_type = T;

  template<typename U>
  using when_mutable_t = U;
};

template<typename T>
class Tree {
  struct Node {
    typedef bool color_t;
    static constexpr color_t Black = false;
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
        _max_offset(0),
        _data(std::move(data))
    {}

    /// Construct a node by moving out of \c other. The color defaults to black.
    Node(Node &&other) noexcept(std::is_nothrow_move_constructible_v<T>)
      : _parent(other.parent()),
        _left(other.left()),
        _right(other.right()),
        _offset(other.offset()),
        _length(other.length()),
        _min_offset(other.min_offset()),
        _max_offset(other.max_offset()),
        _data(std::move(other).data())
    {
      other.unlink();
    }

    /// Move \c other into this node. Delete's this node's previous children
    /// if they exist. Does not adjust this node's color.
    Node &operator=(Node &&other)
      noexcept(std::is_nothrow_move_assignable_v<T>)
    {
      set_parent(other.parent());

      if (left()) {
        delete left();
      }
      set_left(other.left());

      if (right()) {
        delete right();
      }
      set_right(other.right());

      other.unlink();

      set_offset(other.offset());
      set_length(other.length());
      set_min_max(other.min_offset(), other.max_offset());
      set_data(std::move(other).data());

      return *this;
    }

#if 0
    /// Construct a node by copying from \c other. Makes a deep recursive copy
    /// of the entire subtree. The new node's parent is empty (it is a root
    /// node).
    Node(const Node &other)
      : _parent(nullptr),
        _offset(other.offset()),
        _length(other.length()),
        _max_length(other.max_length()),
        _data(other.data())
    {
      const Node *other_parent = other.parent();
      while (other_parent) {
        set_offset(offset() + other_parent->offset());
        other_parent = other_parent->parent();
      }

      if (other.left()) {
        // Temporarily erase parent so we don't do a recursive offset
        // calculation for every node.
        // Note: other is const so this won't work...
        auto parent_tmp = other.left()->parent();
        other.left->set_parent(nullptr);

        set_left(new Node(*other.left()));
        left()->set_parent(this);

        other.left()->set_parent(parent_tmp);
      } else {
        set_left(nullptr);
      }

      if (other.right()) {
        // Prevent recursive offset calculation.
        auto parent_tmp = other.right()->parent();
        other.right()->set_parent(nullptr);

        set_right(new Node(*other.right()));
        right()->set_parent(this);

        other.right()->set_parent(parent_tmp);
      } else {
        set_right(nullptr);
      }
    }

    Node &operator=(const Node &other) {
      set_parent(nullptr);

      set_offset(other.offset);
      const Node *other_parent = other.parent();
      while (other_parent) {
        set_offset(offset() + other_parent->offset());
        other_parent = other_parent->parent();
      }

      if (other.left()) {
        // Prevent recursive offset calculation.
        auto parent_tmp = other.left()->parent();
        other.left()->set_parent(nullptr);

        set_left(new Node(*other.left()));
        left()->set_parent(this);

        other.left()->set_parent(parent_tmp);
      } else {
        set_left(nullptr);
      }

      if (other.right()) {
        // Prevent recursive offset calculation.
        auto parent_tmp = other.right()->parent();
        other.right()->set_parent(nullptr);

        set_right(new Node(*other.right()));
        right()->set_parent(this);

        other.right()->set_parent(parent_tmp);
      } else {
        set_right(nullptr);
      }

      set_length(other.length());
      set_max_length(other.max_length());
      set_data(other.data());

      return *this;
    }
#endif

    ~Node() {
      if (left()) {
        delete left();
      }
      if (right()) {
        delete right();
      }
    }

    void unlink() {
      set_parent(nullptr);
      set_left(nullptr);
      set_right(nullptr);
    }

    bool update_min_max() {
      ptrdiff_t my_min, my_max;
      ptrdiff_t left_min, left_max;
      ptrdiff_t right_min, right_max;

      my_min = left_min = right_min = left_max = right_max = 0;
      my_max = length();

      if (left()) {
        left_min = left()->min_offset();
        left_max = left()->max_offset();
      }

      if (right()) {
        right_min = right()->min_offset();
        right_max = right()->max_offset();
      }

      auto min = std::min({my_min, left_min, right_min}) + offset();
      auto max = std::max({my_max, left_max, right_max}) + offset();

      if (min != _min_offset || max != _max_offset) {
        set_min_max(min, max);
        return true;
      }

      return false;
    }

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
        if (_max_offset + parent->_offset > parent->_max_offset ||
            _min_offset + parent->_offset < parent->_min_offset) {
          parent->update_min_max_recursive<true>();
        }
      } else {
        if (_max_offset + parent->_offset < parent->_max_offset ||
            _min_offset + parent->_offset > parent->_min_offset) {
          parent->update_min_max_recursive<false>();
        }
      }
    }

    void fix_for_insert(Tree *tree) {
      auto parent = this->parent();

      if (!parent) {
        set_color(Black);
        return;
      }

      set_color(Red);

      if (parent->color() == Black) {
        return;
      }

      // We know there's a grandparent because parent is red, which the root
      // cannot be.
      auto grandparent = parent->parent();
      bool parent_is_left = parent == grandparent->left();
      auto uncle = parent_is_left ? grandparent->right() : grandparent->left();
      if (uncle && uncle->color() == Red) {
        parent->set_color(Black);
        uncle->set_color(Black);
        grandparent->set_color(Red);
        grandparent->fix_for_insert(tree);
        return;
      }

      if (this == parent->right() && parent_is_left) {
        tree->rotate_left(parent);
        parent->fix_for_insert_rotate(tree);
      } else if (this == parent->left() && !parent_is_left) {
        tree->rotate_right(parent);
        parent->fix_for_insert_rotate(tree);
      } else {
        this->fix_for_insert_rotate(tree);
      }
    }

  private:
    // Assumes parent is red, grandparent is valid.
    void fix_for_insert_rotate(Tree *tree) {
      auto parent = this->parent();
      auto grandparent = parent->parent();

      if (this == parent->left()) {
        tree->rotate_right(grandparent);
      } else {
        tree->rotate_left(grandparent);
      }

      parent->set_color(Black);
      grandparent->set_color(Red);
    }

  public:
    void fix_for_remove() {
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
      const size_t pos = _offset + static_cast<ptrdiff_t>(parent_pos);

      const size_t min = min_offset() + parent_pos,
                   max = max_offset() + parent_pos;

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
    /// Minimum offset of subtree.
    ptrdiff_t _min_offset;
    /// Maximum offset of subtree.
    ptrdiff_t _max_offset;
    /// Interval length and node color.
    size_t _length;
    /// Associated data.
    T _data;
  };

public:
  // Standard in-order iterator.
  template<bool IsConst>
  class Iterator {
  protected:
    // Node type with appropriate const-ness.
    using node_type = typename maybe_const<Node *, IsConst>::type;

    // Allow const iterator to access members for converting constructor and
    // assignment operator.
    friend class Iterator<true>;

    // Current node pointed to by this iterator.
    node_type _node;

    // Position of the current node.
    size_t _position;

  public:
    explicit Iterator(Node *node, size_t position) noexcept
      : _node(node),
        _position(position + node->offset())
    {}

    // End iterator constructor.
    explicit Iterator() noexcept
      : _node(nullptr),
        _position(0)
    {}

  private:
    static bool no_condition(node_type, size_t) {
      return true;
    }

  protected:
    template<typename F,
             typename = std::is_invocable_r<bool, F, const Node *, size_t>>
    [[nodiscard]] bool move_up_if(const F &condition) {
      auto parent = _node->parent();
      size_t parent_position = static_cast<ptrdiff_t>(_position)
                               - _node->offset();
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

    template<typename F,
             typename = std::is_invocable_r<bool, F, node_type, size_t>>
    [[nodiscard]] bool move_left_if(const F &condition) {
      auto left = _node->left();
      if (!left) {
        return false;
      }
      size_t left_position = static_cast<ptrdiff_t>(_position)
                             + left->offset();
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

    template<typename F,
             typename = std::is_invocable_r<bool, F, node_type, size_t>>
    [[nodiscard]] bool move_right_if(const F &condition) {
      auto right = _node->right();
      if (!right) {
        return false;
      }
      size_t right_position = static_cast<ptrdiff_t>(_position)
                              + right->offset();
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

    template<typename F,
             typename = std::is_invocable_r<bool, F, node_type, size_t>>
    [[nodiscard]] bool move_next_if(const F &condition) {
      auto node = _node;
      auto position = _position;

      if (node->right() &&
          condition(node->right(), position + node->right()->offset())) {
        // If there is a right subtree, go all the way to its left side.
        node = node->right();
        position += node->offset();
        while (node->left() &&
               condition(node->left(), position + node->left()->offset())) {
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
            !condition(node->parent(), position - node->offset())) {
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

    template<typename F,
             typename = std::is_invocable_r<bool, F, node_type, size_t>>
    [[nodiscard]] bool move_prev_if(const F &condition) {
      auto node = _node;
      auto position = _position;

      // Do the same as move_next_if, but in reverse.
      if (node->left() &&
          condition(node->left(), position + node->left()->offset())) {
        node = node->left();
        position += node->offset();
        while (node->right() &&
               condition(node->right(), position + node->right()->offset())) {
          node = node->right();
          position += node->offset();
        }
      } else {
        while (node->parent() && node == node->parent()->left()) {
          position -= node->offset();
          node = node->parent();
        }
        if (!node->parent() ||
            !condition(node->parent(), position - node->offset())) {
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
    typedef typename maybe_const<T *, IsConst>::type pointer;
    typedef typename maybe_const<T &, IsConst>::type reference;
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

    Iterator &operator++() {
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

    Iterator &operator--() {
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
    Class &operator++() {                                                      \
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
    Class &operator--() {                                                      \
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

    bool is_possible_search_node(const Node *node, size_t pos) const {
      const size_t search_length = _end - _start;
      const size_t parent_pos = pos - node->offset();

      return node->length() >= search_length &&
             node->min_offset() + parent_pos <= _start &&
             node->max_offset() + parent_pos >= _end;
    }

    bool is_match() const {
      return _position <= _start &&
             _position + _node->length() >= _end;
    }

    void find_first() {
      auto const search_length = _end - _start;

      // First find the subtree with lengths greater or equal to the search
      // length.
      while (true) {
        auto const node_length = _node->length();
        if (node_length >= search_length) {
          break;
        }

        if (!move_right()) {
          // No match found.
          _node = nullptr;
          return;
        }
      }

      // Look down the left side until max - min is too small.
      node_type matching_node = nullptr;
      size_t matching_position;
      auto const is_possible_search_node =
        [this](const Node * node, size_t pos) {
          return this->is_possible_search_node(node, pos);
        };

      while (move_left_if(is_possible_search_node)) {
        if (is_match()) {
          matching_node = _node;
          matching_position = _position;
        }
      }
      if (matching_node) {
        _node = matching_node;
        _position = matching_position;
        return;
      }

      // Is it to the right?
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

    bool is_possible_search_node(const Node *node, size_t pos) {
      const size_t parent_pos = pos - node->offset();

      return node->min_offset() + parent_pos < _end &&
             node->max_offset() + parent_pos > _start;
    }

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

      // Look down the left side until min is too high or max is too low.
      node_type matching_node = nullptr;
      size_t matching_position;
      auto const is_possible_search_node =
        [this](const Node * node, size_t pos) {
          return this->is_possible_search_node(node, pos);
        };

      while (move_left_if(is_possible_search_node)) {
        if (is_match()) {
          matching_node = _node;
          matching_position = _position;
        }
      }
      if (matching_node) {
        _node = matching_node;
        _position = matching_position;
        return;
      }

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

    bool is_possible_search_node(const Node *node, size_t pos) const {
      const size_t parent_pos = pos - node->offset();
      const size_t min_pos = node->min_offset() + parent_pos;
      const size_t max_pos = node->max_offset() + parent_pos;
      
      return min_pos < _end && max_pos > _start;
    }

    bool is_match() const {
      return _position < _end && _position + _node->length() > _start;
    }

    void find_first() {
      auto const is_possible_search_node =
        [this](const Node * node, size_t pos) {
          return this->is_possible_search_node(node, pos);
        };

      // We have to search everything that meets is_possible_search_node.
      // Just start from the left, and if we don't find anything start looking
      // at the right subtree.
      node_type matching_node = nullptr;
      size_t matching_position;

      while (move_left_if(is_possible_search_node)) {
        if (is_match()) {
          matching_node = _node;
          matching_position = _position;
        }
      }
      if (matching_node) {
        _node = matching_node;
        _position = matching_position;
        return;
      }

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
      const size_t search_length = _end - _start;
      const size_t parent_pos = pos - node->offset();
      const size_t min_pos = node->min_offset() + parent_pos;
      const size_t max_pos = node->max_offset() + parent_pos;

      if (node->length() != search_length) {
        return false;
      }
      if (min_pos > _start || max_pos < _end) {
        return false;
      }

      return true;
    }

    bool is_match() const {
      // is_possible_search_node already made sure lengths are equal.
      return _position == _start;
    }

    void find_first() {
      const size_t search_length = _end - _start;
      // First find the subtree with matching length, if it exists.
      while (true) {
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
      // Min must be <= _start and max must be >= _end.
      size_t parent_pos = _position - _node->offset();
      if (static_cast<size_t>(_node->min_offset() + parent_pos) > _start ||
          static_cast<size_t>(_node->max_offset() + parent_pos) < _end) {
        // The whole subtree is out of range, so there is no matching node.
        _node = nullptr;
        return;
      }

      node_type matching_node = nullptr;
      auto const is_possible_search_node =
        [this](const Node * node, size_t pos) {
          return this->is_possible_search_node(node, pos);
        };

      // First try to find the node in the left subtree.
      while (move_left_if(is_possible_search_node)) {
        if (is_match()) {
          matching_node = _node;
        }
      }
      if (matching_node) {
        _node = matching_node;
        _position = _start;
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
  iterator begin() {
    return iterator();
  }

  const_iterator begin() const {
    return const_iterator();
  }

  const_iterator cbegin() const {
    return begin();
  }

  iterator end() {
    return iterator();
  }

  const_iterator end() const {
    return const_iterator();
  }

  const_iterator cend() const {
    return end();
  }

  outer_search_iterator find_outer(size_t start, size_t end) {
    return outer_search_iterator(_root, 0, start, end);
  }

  const_outer_search_iterator find_outer(size_t start, size_t end) const {
    return const_outer_search_iterator(_root, 0, start, end);
  }

  inner_search_iterator find_inner(size_t start, size_t end) {
    return inner_search_iterator(_root, 0, start, end);
  }

  const_inner_search_iterator find_inner(size_t start, size_t end) const {
    return const_inner_search_iterator(_root, 0, start, end);
  }

  overlap_search_iterator find_overlap(size_t start, size_t end) {
    return overlap_search_iterator(_root, 0, start, end);
  }

  const_overlap_search_iterator find_overlap(size_t start, size_t end) const {
    return const_overlap_search_iterator(_root, 0, start, end);
  }

  equal_search_iterator find_equal(size_t start, size_t end) {
    return equal_search_iterator(_root, 0, start, end);
  }

  const_equal_search_iterator find_equal(size_t start, size_t end) const {
    return const_equal_search_iterator(_root, 0, start, end);
  }

  T &insert(size_t position, size_t length, T &&data) {
    ptrdiff_t offset = static_cast<ptrdiff_t>(position);
    if (!_root) {
      _root = new Node(nullptr, offset, length, std::move(data));
      return _root->data();
    }

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

    node->fix_for_insert(this);
    node->update_min_max_recursive<true>();

    return node->data();
  }

  void remove(const_iterator position) {
  }

  void print() {
    if (!_root) {
      return;
    }
    _root->print(0, 0);
    std::cout << "\n\n\n\n\n\n\n\n\n\n\n\n\n\n\n";
  }

private:
  void fix_for_rotate(Node *old_pivot, Node *new_pivot, Node *parent) {
    auto old_pivot_offset = old_pivot->offset();
    auto new_pivot_offset = new_pivot->offset();

    old_pivot->set_offset(-new_pivot_offset);
    new_pivot->set_offset(old_pivot_offset + new_pivot_offset);

    new_pivot->set_min_max(old_pivot->min_offset(), old_pivot->max_offset());
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

  tree.insert(1, 5, 0);
  tree.insert(2, 3, 0);
  tree.insert(4, 7, 0);
  tree.insert(3, 9, 0);
  tree.insert(2, 4, 0);
  tree.insert(1, 9, 0);
  tree.insert(4, 5, 0);
  tree.insert(2, 6, 0);
  tree.insert(8, 9, 0);
  tree.insert(5, 8, 0);
  tree.insert(5, 9, 0);
  tree.insert(1, 2, 0);

  tree.print();

  std::string inp;
  std::cout << "Search modes: [ou]tside, [i]nside, [ov]erlap, [e]qual.\n";
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
      }
    }
    std::cout << mode << "\n";
    if (mode == 0) {
      break;
    }
    std::cout << "Start> ";
    std::getline(std::cin, inp);
    int start = atoi(inp.c_str());
    std::cout << "End  > ";
    std::getline(std::cin, inp);
    int end = atoi(inp.c_str());

    auto printmatches = [&tree](auto it) {
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
    default:
      __assume(0);
    }
  }

  SetConsoleMode(hstdout, outmode);

  return 0;
}
