#ifndef ASH_IMPLEMENT_SEARCH_ITERATOR
#error This file should be included by intervaltree/searchiterator.h
#endif

// This iterator finds ranges that are equal to the search range.
template<typename T>
class EqualSearchIterator final : public SearchIteratorBase<T> {
  ASH_IMPLEMENT_SEARCH_ITERATOR(EqualSearchIterator)

  // Rule out any nodes whose position is different from the start position or
  // whose max is too small.
  bool is_possible_search_node(const Key<T> &key) const {
    if (key.start_pos() != this->_start ||
        key.node()->max_pos(key.start_pos()) < this->_end) {
      return false;
    }

    return true;
  }

  // A match has to start and end exactly on the search range. We only need to
  // compare the end position, since we will only be looking at nodes with
  // equal start positions.
  bool is_match() const {
    return this->_key.end_pos() == this->_end;
  }

  void find_first() {
    // Find the top-most node with a matching start position.
    while (true) {
      if (this->_start < this->_key.start_pos()) {
        if (!this->move_left()) {
          this->_key = nullptr;
          return;
        }
      } else if (this->_start > this->_key.start_pos()) {
        if (!this->move_right()) {
          this->_key = nullptr;
          return;
        }
      } else {
        break;
      }
    }

    // If the whole subtree is too small, since this is the top-most node,
    // there is no match in the tree.
    if (this->_key.node()->max_pos(this->_key.start_pos()) < this->_end) {
      this->_key = nullptr;
      return;
    }

    using namespace std::placeholders;
    auto const is_possible_search_node = std::bind(
      &EqualSearchIterator::is_possible_search_node,
      this, _1
    );

    // There may be more than one node with the same start position, so first
    // move as far left as possible.
    while (this->move_left_if(is_possible_search_node)) {
    }

    // If we found it, this is the left-most one.
    if (is_match()) {
      return;
    }

    // If it wasn't found, look to the right.
    while (this->move_next_if(is_possible_search_node)) {
      if (is_match()) {
        return;
      }
    }

    // No match found.
    this->_key = nullptr;
  }
};
