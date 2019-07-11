#ifndef ASH_IMPLEMENT_SEARCH_ITERATOR
#error This file should be included by intervaltree/searchiterator.h
#endif

// This iterator finds ranges that are completely inside the search range.
template<typename T>
class InnerSearchIterator final : public SearchIteratorBase<T> {
  ASH_IMPLEMENT_SEARCH_ITERATOR(InnerSearchIterator)

  // Based on starting at the lowest possible start and moving to eligible
  // higher nodes.
  bool is_possible_search_node(const Key<T> &key) {
    if (key.start_pos() < _start) {
      return key.node()->max_pos(key.start_pos()) > _start;
    }
    // If the current key is valid, we have to look at the next node.
    return _key.start_pos() < _end;
  }

  // Is the current interval inside the search interval?
  bool is_match() const {
    return _key.start_pos() >= _start && _key.end_pos() <= _end;
  }

  void find_first() {
    // Find the left-most node that starts inside the search interval.
    while (true) {
      // On the left side of start: must move right if possible.
      if (_key.start_pos() < _start) {
        if (move_right_if([=](const Key<T> &key) {
                            return key.node()->max_pos(key.start_pos()) >
                                   _start;
                          })) {
          continue;
        }
        _key = nullptr;
        return;
      }

      // On the right side of end: must move left.
      if (_key.start_pos() >= _end) {
        if (move_left()) {
          continue;
        }
        _key = nullptr;
        return;
      }

      // Inside the interval. Move as far left as possible.
      if (!move_left_if([=](const Key<T> &key) {
                          return key.start_pos() >= _start;
                        })) {
        break;
      }
    }

    // We're inside the interval. Recursively search the left subtree, then look
    // at this, then the right subtree.
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
