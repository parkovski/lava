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
    return _key.start_pos() < _end &&
           key.node()->max_pos(key.start_pos()) > _start;
  }

  // A match is any node that starts before the end of the search range and
  // ends after its start. We ruled out anything starting after the search
  // end already, so just compare to the start.
  bool is_match() const {
    return _key.start_pos() < _end && _key.end_pos() > _start;
  }

  void find_first() {
    // Find the top-most node with an acceptable start and max.
    while (true) {
      if (_key.start_pos() >= _end) {
        if (move_left()) {
          continue;
        }
        _key = nullptr;
        return;
      }

      if (_key.node()->max_pos(_key.start_pos()) < _start) {
        _key = nullptr;
        return;
      }

      break;
    }

    // Find the left-most match.
    // TODO: This should be a BFS search, but operator++ needs to be modified.
    auto top = _key;
    if (move_left()) {
      find_first();
      if (_key) {
        return;
      }
    }
    _key = top;
    if (is_match()) {
      return;
    }
    if (move_right()) {
      find_first();
    } else {
      _key = nullptr;
    }
  }
};
