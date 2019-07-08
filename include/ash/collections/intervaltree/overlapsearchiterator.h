#ifndef ASH_IMPLEMENT_SEARCH_ITERATOR
#error This file should be included by intervaltree/searchiterator.h
#endif

// This iterator finds ranges that overlap the search range at some point.
template<typename T>
class OverlapSearchIterator final : public SearchIteratorBase<T> {
  ASH_IMPLEMENT_SEARCH_ITERATOR(OverlapSearchIterator)

  // For overlapping nodes, we have to look at any node starting before end
  // with a max after start.
  bool is_possible_search_node(const Key<T> &key) const {
    return key.start_pos() < _end &&
           key.node()->max_pos(key.start_pos()) > _start;
  }

  // A match is any node that starts before the end of the search range and
  // ends after its start. We ruled out anything starting after the search
  // end already, so just compare to the start.
  bool is_match() const {
    return _key.end_pos() > _start;
  }

  void find_first() {
    using namespace std::placeholders;
    auto const is_possible_search_node = std::bind(
      &OverlapSearchIterator::is_possible_search_node,
      this, _1
    );

    // Move to the node with the smallest possible max.
    while (move_left_if([=](const Key<T> &key) {
                          return key.node()->max_pos(key.start_pos()) > _start;
                        })) {
    }

    // If the start position is too high, there are no matches.
    if (_key.start_pos() >= _end) {
      _key = nullptr;
      return;
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
    _key = nullptr;
  }
};
