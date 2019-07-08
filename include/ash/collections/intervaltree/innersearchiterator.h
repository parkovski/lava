#ifndef ASH_IMPLEMENT_SEARCH_ITERATOR
#error This file should be included by intervaltree/searchiterator.h
#endif

// This iterator finds ranges that are completely inside the search range.
template<typename T>
class InnerSearchIterator final : public SearchIteratorBase<T> {
  ASH_IMPLEMENT_SEARCH_ITERATOR(InnerSearchIterator)

  // Subtrees of interest have a start position between the search range.
  // We only need to compare against the end since we skip everything before
  // start.
  bool is_possible_search_node(const Key<T> &key) {
    return key.start_pos() <= _end;
  }

  // A match starts at or after the search start and ends at or before the
  // search end. We already ruled out nodes left of the search start, so
  // just look at the end.
  bool is_match() const {
    return _key.end_pos() <= _end;
  }

  void find_first() {
    // Find the top-most node that starts inside the search interval.
    while (true) {
      if (_key.start_pos() < _start) {
        if (!move_right()) {
          _key = nullptr;
          return;
        }
      } else if (_key.start_pos() > _end) {
        if (!move_left()) {
          _key = nullptr;
          return;
        }
      } else {
        break;
      }
    }

    // Find the node that starts closest to the search start.
    while (move_left_if([=](const Key<T> &key) {
                          return key.start_pos() >= _start; })) {
    }

    // This would be the minimum match.
    if (is_match()) {
      return;
    }

    using namespace std::placeholders;
    auto const is_possible_search_node = std::bind(
      &InnerSearchIterator::is_possible_search_node,
      this, _1
    );

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
