#ifndef ASH_IMPLEMENT_SEARCH_ITERATOR
#error This file should be included by intervaltree/searchiterator.h
#endif

// This iterator finds ranges that overlap the search range at some point.
template<typename T>
class OverlapSearchIterator final : public SearchIteratorBase<T> {
  ASH_IMPLEMENT_SEARCH_ITERATOR(OverlapSearchIterator)

  // For overlapping nodes, length doesn't matter and we have to look at
  // any node with a min before end and a max after start.
  bool is_possible_search_node(const Key<T> &key) const {
    return key.node()->min_pos(key.begin()) < _end &&
           key.node()->max_pos(key.begin()) > _start;
  }

  // A match is any node that starts before the end of the search range and
  // ends after its start.
  bool is_match() const {
    return _key.begin() < _end &&
           _key.end()   > _start;
  }

  void find_first() {
    using namespace std::placeholders;
    auto const is_possible_search_node = std::bind(
      &OverlapSearchIterator::is_possible_search_node,
      this, _1
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
    _key = nullptr;
  }
};
