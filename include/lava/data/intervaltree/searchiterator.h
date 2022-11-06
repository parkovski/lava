#ifndef LAVA_DATA_INTERVALTREE_SEARCHITERATOR_H_
#define LAVA_DATA_INTERVALTREE_SEARCHITERATOR_H_

#include "iterator.h"
#include <functional>

namespace lava::data::itree {

using namespace detail;

template<typename T>
class SearchIteratorBase : public Iterator<T> {
  friend class SearchIteratorBase<opposite_const_t<T>>;

protected:
  using typename Iterator<T>::difference_type;
  using typename Iterator<T>::value_type;
  using typename Iterator<T>::pointer;
  using typename Iterator<T>::reference;
  using typename Iterator<T>::iterator_category;

  /// \param node The node to start searching from.
  /// \param position The position of \c node.
  /// \param start The start of the search range.
  /// \param end The end of the search range.
  explicit SearchIteratorBase(node_t<T> *node, size_t position,
                              size_t start, size_t end) noexcept
    : Iterator<T>(node, position),
      _start(start),
      _end(end)
  {}

public:
  SearchIteratorBase(const SearchIteratorBase &) = default;

  // Construct a SearchIteratorBase<const T> from a SearchIteratorBase<T>.
  template<typename U = T>
  SearchIteratorBase(
    const SearchIteratorBase<std::remove_const_t<if_const_t<U>>> &other
  )
    noexcept
    : Iterator<T>(other._node, other._position),
      _start(other._start),
      _end(other._end)
  {}

  SearchIteratorBase &operator=(const SearchIteratorBase &) = default;

  // Assign to a SearchIteratorBase<const T> from a SearchIteratorBase<T>.
  template<typename U = T>
  SearchIteratorBase &
  operator=(
    const SearchIteratorBase<std::remove_const_t<if_const_t<U>>> &other
  ) noexcept {
    Iterator<T>::operator=(other);
    _start = other._start;
    _end = other._end;
    return *this;
  }

protected:
  size_t _start;
  size_t _end;
};

#define LAVA_IMPLEMENT_SEARCH_ITERATOR(Class)                                   \
  public:                                                                      \
    friend class Class<opposite_const_t<T>>;                                   \
                                                                               \
    using typename SearchIteratorBase<T>::difference_type;                     \
    using typename SearchIteratorBase<T>::value_type;                          \
    using typename SearchIteratorBase<T>::pointer;                             \
    using typename SearchIteratorBase<T>::reference;                           \
    using typename SearchIteratorBase<T>::iterator_category;                   \
                                                                               \
    explicit Class(node_t<T> *node, size_t position, size_t start,             \
                   size_t end) noexcept                                        \
      : SearchIteratorBase<T>(node, position, start, end)                      \
    {                                                                          \
      if (node) {                                                              \
        find_first();                                                          \
      }                                                                        \
    }                                                                          \
                                                                               \
    /* Copy constructor */                                                     \
    Class(const Class &other) noexcept                                         \
      : SearchIteratorBase<T>(other)                                           \
    {}                                                                         \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename U = T>                                                   \
    Class(const Class<std::remove_const_t<if_const_t<U>>> &other) noexcept     \
      : SearchIteratorBase<T>(other)                                           \
    {}                                                                         \
                                                                               \
    /* Copy assignment */                                                      \
    Class &operator=(const Class &other) noexcept {                            \
      SearchIteratorBase<T>::operator=(other);                                 \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    /* Convert from iterator to const_iterator. */                             \
    template<typename U = T>                                                   \
    Class &operator=(const Class<std::remove_const_t<if_const_t<U>>> &other)   \
      noexcept                                                                 \
    {                                                                          \
      SearchIteratorBase<T>::operator=(other);                                 \
      return *this;                                                            \
    }                                                                          \
                                                                               \
    Class &operator++() override {                                             \
      auto const is_possible_search_node =                                     \
        [this](const Key<T> &key) {                                            \
          return this->is_possible_search_node(key);                           \
        };                                                                     \
                                                                               \
      while (this->move_next_if(is_possible_search_node)) {                    \
        if (is_match()) {                                                      \
          return *this;                                                        \
        }                                                                      \
      }                                                                        \
                                                                               \
      /* Nothing found. */                                                     \
      this->_key = nullptr;                                                    \
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
      while (this->move_prev_if(is_possible_search_node)) {                    \
        if (is_match()) {                                                      \
          return *this;                                                        \
        }                                                                      \
      }                                                                        \
                                                                               \
      this->_key = nullptr;                                                    \
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

//#include "outersearchiterator.h"
#include "innersearchiterator.h"
#include "overlapsearchiterator.h"
#include "equalsearchiterator.h"

#undef LAVA_IMPLEMENT_SEARCH_ITERATOR

} // namespace lava::data::itree

#endif // LAVA_DATA_INTERVALTREE_SEARCHITERATOR_H_
