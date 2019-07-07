#ifndef ASH_COLLECTIONS_INTERVALTREE_SEARCHITERATOR_H_
#define ASH_COLLECTIONS_INTERVALTREE_SEARCHITERATOR_H_

#include "iterator.h"

namespace ash::collections::itree {

template<typename T>
class SearchIteratorBase : public Iterator<T> {
  friend class SearchIteratorBase<opposite_const_t<T>>;

protected:
  using typename Iterator::difference_type;
  using typename Iterator::value_type;
  using typename Iterator::pointer;
  using typename Iterator::reference;
  using typename Iterator::iterator_category;

  /// \param node The node to start searching from.
  /// \param position The position of \c node.
  /// \param start The start of the search range.
  /// \param end The end of the search range.
  explicit SearchIteratorBase(node_t<T> *node, size_t position,
                              size_t start, size_t end) noexcept
    : Iterator(node, position),
      _start(start),
      _end(end)
  {}

public:
  SearchIteratorBase(const SearchIteratorBase &) = default;

  // Construct a SearchIteratorBase<const T> from a SearchIteratorBase<T>.
  template<typename = if_const_t<T>>
  SearchIteratorBase(const SearchIteratorBase<std::remove_const_t<T>> &other)
    noexcept
    : Iterator(other._node, other._position),
      _start(other._start),
      _end(other._end)
  {}

  SearchIteratorBase &operator=(const SearchIteratorBase &) = default;

  // Assign to a SearchIteratorBase<const T> from a SearchIteratorBase<T>.
  template<typename = if_const_t<T>>
  SearchIteratorBase &
  operator=(const SearchIteratorBase<std::remove_const_t<T>> &other) noexcept {
    Iterator::operator=(other);
    _start = other._start;
    _end = other._end;
    return *this;
  }

protected:
  size_t _start;
  size_t _end;
};

#define ASH_IMPLEMENT_SEARCH_ITERATOR(Class)                                   \
  public:                                                                      \
    friend class Class<opposite_const_t<T>>;                                   \
                                                                               \
    using SearchIteratorBase::difference_type;                                 \
    using SearchIteratorBase::value_type;                                      \
    using SearchIteratorBase::pointer;                                         \
    using SearchIteratorBase::reference;                                       \
    using SearchIteratorBase::iterator_category;                               \
                                                                               \
    explicit Class(node_t<T> *node, size_t position, size_t start,             \
                   size_t end) noexcept                                        \
      : SearchIteratorBase(node, position, start, end)                         \
    {                                                                          \
      if (node) {                                                              \
        find_first();                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    /* Copy constructor */                                                     \
    Class(const Class &other) noexcept                                         \
      : SearchIteratorBase(other)                                              \
    {}                                                                         \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename = if_const_t<T>>                                         \
    Class(const Class<std::remove_const_t<T>> &other) noexcept                 \
      : SearchIteratorBase(other)                                              \
    {}                                                                         \
                                                                               \
    /* Copy assignment */                                                      \
    Class &operator=(const Class &other) noexcept {                            \
      SearchIteratorBase::operator=(other);                                    \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename = if_const_t<T>>                                         \
    Class &operator=(const Class<std::remove_const_t<T>> &other) noexcept {    \
      SearchIteratorBase::operator=(other);                                    \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    /* All search iterators may be constructed or assigned from another. */    \
    Class(const SearchIteratorBase &other) noexcept                            \
      : SearchIteratorBase(other)                                              \
    {                                                                          \
      if (!is_match()) {                                                       \
        find_first();                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename = if_const_t<T>>                                         \
    Class(const SearchIteratorBase<std::remove_const_t<T>> &other) noexcept    \
      : SearchIteratorBase(other)                                              \
    {                                                                          \
      if (!is_match()) {                                                       \
        find_first();                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    /* All search iterators may be constructed or assigned from another. */    \
    Class &operator=(const SearchIteratorBase &other) noexcept {               \
      SearchIteratorBase::operator=(other);                                    \
      if (!is_match()) {                                                       \
        find_first();                                                          \
      }                                                                        \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename = if_const_t<T>>                                         \
    Class &operator=(const SearchIteratorBase<std::remove_const_t<T>> &other)  \
      noexcept {                                                               \
      SearchIteratorBase::operator=(other);                                    \
      if (!is_match()) {                                                       \
        find_first();                                                          \
      }                                                                        \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    Class &operator++() override {                                             \
      auto const is_possible_search_node =                                     \
        [this](const Key<T> &key) {                                            \
          return this->is_possible_search_node(key);                           \
        };                                                                     \
                                                                               \
      while (move_next_if(is_possible_search_node)) {                          \
        if (is_match()) {                                                      \
          return *this;                                                        \
        }                                                                      \
      }                                                                        \
                                                                               \
      /* Nothing found. */                                                     \
      _key = nullptr;                                                          \
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
        [this](const Key<T> &key) {                                            \
          return this->is_possible_search_node(key);                           \
        };                                                                     \
                                                                               \
      while (move_prev_if(is_possible_search_node)) {                          \
        if (is_match()) {                                                      \
          return *this;                                                        \
        }                                                                      \
      }                                                                        \
                                                                               \
      _key = nullptr;                                                          \
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

#include "outersearchiterator.h"
#include "innersearchiterator.h"
#include "overlapsearchiterator.h"
#include "equalsearchiterator.h"

#undef ASH_IMPLEMENT_SEARCH_ITERATOR

} // namespace ash::collections::itree

#endif // ASH_COLLECTIONS_INTERVALTREE_SEARCHITERATOR_H_
