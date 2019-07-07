#ifndef ASH_IMPLEMENT_SEARCH_ITERATOR
#error This file should be included by intervaltree/searchiterator.h
#endif

// This iterator finds ranges that are fully outside the search range. That
// is, all returned ranges fully overlap the search range.
template<typename T>
class OuterSearchIterator final : public SearchIteratorBase<T> {
  ASH_IMPLEMENT_SEARCH_ITERATOR(OuterSearchIterator)

  // For an outer range search, we only care about the subtree with lengths
  // of at least the search length, where the min is at or before the start
  // and the max is at or after the end.
  bool is_possible_search_node(const Key<T> &key) const {
    const size_t search_length = _end - _start;

    return key.length() >= search_length &&
           key.node()->min_pos(key.begin()) <= _start &&
           key.node()->max_pos(key.begin()) >= _end;
  }

  // Checks whether the current node fully covers the search range.
  bool is_match() const {
    return _key.begin() <= _start &&
           _key.end()   >= _end;
  }

  // Find the shortest (left-most) interval that satisfies the search.
  void find_first() {
    auto const search_length = _end - _start;

    // First find the subtree with lengths greater or equal to the search
    // length.
    while (_key.length() < search_length) {
      if (!move_right()) {
        // No match found.
        _key = nullptr;
        return;
      }
    }

    using namespace std::placeholders;
    auto const is_possible_search_node = std::bind(
      &OuterSearchIterator::is_possible_search_node,
      this, _1
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
    _key = nullptr;
  }
};
