#ifndef ASH_IMPLEMENT_SEARCH_ITERATOR
#error This file should be included by intervaltree/searchiterator.h
#endif

// This iterator finds ranges that are completely inside the search range.
template<typename T>
class InnerSearchIterator final : public SearchIteratorBase<T> {
  ASH_IMPLEMENT_SEARCH_ITERATOR(InnerSearchIterator)

  // Subtrees of interest have a length no greater than the search length,
  // a min before the end of the search range, and a max before its start.
  bool is_possible_search_node(const Key<T> &key) {
    if (key.length() > _end - _start) {
      return false;
    }

    return key.node()->min_pos(key.begin()) < _end &&
           key.node()->max_pos(key.begin()) > _start;
  }

  // A match starts at or after the search start and ends at or before the
  // search end.
  bool is_match() const {
    return _key.begin() >= _start &&
           _key.end()   <= _end;
  }

  void find_first() {
    auto const search_length = _end - _start;

    // First find the subtree with lengths less or equal to the search length.
    while (true) {
      auto const node_length = _key.length();
      if (node_length <= search_length) {
        break;
      }

      if (!move_left()) {
        // No match found.
        _key = nullptr;
        return;
      }
    }

    using namespace std::placeholders;
    auto const is_possible_search_node = std::bind(
      &InnerSearchIterator::is_possible_search_node,
      this, _1
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
    _key = nullptr;
  }
};
