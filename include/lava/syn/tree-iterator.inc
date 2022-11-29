struct const_iterator {
  typedef void difference_type;
  typedef const Node &value_type;
  typedef const Node *pointer;
  typedef const Node &reference;
  typedef std::bidirectional_iterator_tag iterator_category;

  explicit const_iterator(const Tree *parent, unsigned index)
           noexcept
    : _parent{const_cast<Tree*>(parent)}
    , _index{index}
  {}

  const_iterator(const const_iterator &) = default;
  const_iterator &operator=(const const_iterator &) = default;

  friend bool operator==(const const_iterator &a, const const_iterator &b)
              = default;
  friend auto operator<=>(const const_iterator &a, const const_iterator &b)
              = default;

  friend void swap(const_iterator &a, const_iterator &b) noexcept {
    using std::swap;
    swap(a._parent, b._parent);
    swap(a._index, b._index);
  }

  const Node &operator*() const {
    return *_parent->get_child(_index);
  }

  const Node &operator->() const {
    return **this;
  }

  const_iterator &operator++() {
    ++_index;
    return *this;
  }

  const_iterator operator++(int) {
    const_iterator pre = *this;
    ++_index;
    return pre;
  }

  const_iterator &operator--() {
    --_index;
    return *this;
  }

  const_iterator operator--(int) {
    const_iterator pre = *this;
    --_index;
    return pre;
  }

protected:
  Tree *_parent = nullptr;
  unsigned _index = 0;
};

struct iterator : const_iterator {
  using const_iterator::difference_type;
  typedef Node &value_type;
  typedef Node *pointer;
  typedef Node &reference;
  using const_iterator::iterator_category;

  explicit iterator(Tree *parent, unsigned index) noexcept
    : const_iterator{parent, index}
  {}

  iterator(const iterator &) = default;
  iterator &operator=(const iterator &) = default;

  friend bool operator==(const iterator &a, const iterator &b) = default;
  friend auto operator<=>(const iterator &a, const iterator &b) = default;

  friend void swap(iterator &a, iterator &b) {
    swap(static_cast<const_iterator&>(a), static_cast<const_iterator&>(b));
  }

  Node &operator*() {
    return *_parent->get_child(_index);
  }

  Node &operator->() {
    return **this;
  }

  iterator &operator++() {
    ++_index;
    return *this;
  }

  iterator operator++(int) {
    iterator pre = *this;
    ++_index;
    return pre;
  }

  iterator &operator--() {
    --_index;
    return *this;
  }

  iterator operator--(int) {
    iterator pre = *this;
    --_index;
    return pre;
  }
};
// vim: set ft=cpp:
