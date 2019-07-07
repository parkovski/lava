#ifndef ASH_IMPLEMENT_SEARCH_ITERATOR
#error This file should be included by intervaltree/searchiterator.h
#endif

// This iterator finds ranges that are equal to the search range.
template<typename T>
class EqualSearchIterator final : public SearchIteratorBase<T> {
  ASH_IMPLEMENT_SEARCH_ITERATOR(EqualSearchIterator)

  // Rule out any nodes whose length is not equal to the target length
  // or whose min and max are out of range.
  bool is_possible_search_node(const Key<T> &key) const {
    if (key.length() != _end - _start) {
      return false;
    }
    if (key.node()->min_pos(key.begin()) > _start ||
        key.node()->max_pos(key.begin()) < _end) {
      return false;
    }

    return true;
  }

  // A match has to start and end exactly on the search range.
  bool is_match() const {
    return _key.begin() == _start &&
           _key.end()   == _end;
  }

  void find_first() {
    const size_t search_length = _end - _start;

    // First find the subtree with matching length, if it exists, making
    // sure to only look at subtrees with a valid min and max.
    while (_key.node()->min_pos(_key.begin()) <= _start
        && _key.node()->max_pos(_key.begin()) >= _end) {
      auto const node_length = _key.length();
      if (node_length > search_length) {
        if (!move_left()) {
          // No matching length exists.
          _key = nullptr;
          return;
        }
      } else if (node_length < search_length) {
        if (!move_right()) {
          // No matching length exists.
          _key = nullptr;
          return;
        }
      } else {
        // Lengths are equal.
        break;
      }
    }

    // Find the first element with a matching start position.
    using namespace std::placeholders;
    auto const is_possible_search_node = std::bind(
      &EqualSearchIterator::is_possible_search_node,
      this, _1
    );

    // First move as far left as possible.
    while (move_left_if(is_possible_search_node)) {
    }

    // If we found it, this is the left-most one.
    if (is_match()) {
      return;
    }

    // If it wasn't found, look to the right.
    while (move_next_if(is_possible_search_node)) {
      if (is_match()) {
        return;
      }
    }

    // No match found.
    _key = nullptr;
  }
};
