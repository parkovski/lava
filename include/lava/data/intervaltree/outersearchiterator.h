#ifndef LAVA_IMPLEMENT_SEARCH_ITERATOR
#error This file should be included by intervaltree/searchiterator.h
#endif

// This iterator finds ranges that are fully outside the search range. That
// is, all returned ranges fully overlap the search range.
template<typename T>
class OuterSearchIterator final : public SearchIteratorBase<T> {
  LAVA_IMPLEMENT_SEARCH_ITERATOR(OuterSearchIterator)

  // For an outer range search, we only care about nodes starting at or before
  // the search start with a max at or after the search end.
  bool is_possible_search_node(const Key<T> &key) const {
    return key.start_pos() <= _start &&
           key.node()->max_pos(key.start_pos()) >= _end;
  }

  // Checks whether the current node fully covers the search range. We already
  // excluded nodes after the start, so just compare with the end.
  bool is_match() const {
    return _key.end_pos() >= _end;
  }

  // Find the left-most interval that satisfies the search.
  void find_first() {
    auto const search_length = _end - _start;

    // First find the lowest node with a max greater or equal to end.
    while (this->move_left_if([=](const Key<T> &key) {
                          return key.node()->max_pos(key.start_pos()) >= this->_end;
                        })) {
    }

    using namespace std::placeholders;
    auto const is_possible_search_node = std::bind(
      &OuterSearchIterator::is_possible_search_node,
      this, _1
    );

    // Is it the minimum possible?
    if (is_match()) {
      return;
    }

    // Not found yet. Is it to the right?
    while (this->move_next_if(is_possible_search_node)) {
      if (is_match()) {
        return;
      }
    }

    // Didn't find anything.
    _key = nullptr;
  }
};
