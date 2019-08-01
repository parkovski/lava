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
    if (key.start_pos() < this->_start) {
      return key.node()->max_pos(key.start_pos()) > this->_start;
    }
    // If the current key is valid, we have to look at the next node.
    return this->_key.start_pos() < this->_end;
  }

  // Is the current interval inside the search interval?
  bool is_match() const {
    return this->_key.start_pos() >= this->_start &&
           this->_key.end_pos() <= this->_end;
  }

  void find_first() {
    // Find the left-most node that starts inside the search interval.
    while (true) {
      // On the left side of start: must move right if possible.
      if (this->_key.start_pos() < this->_start) {
        if (this->move_right_if([this](const Key<T> &key) {
                            return key.node()->max_pos(key.start_pos()) >
                                   this->_start;
                          })) {
          continue;
        }
        this->_key = nullptr;
        return;
      }

      // On the right side of end: must move left.
      if (this->_key.start_pos() >= this->_end) {
        if (this->move_left()) {
          continue;
        }
        this->_key = nullptr;
        return;
      }

      // Inside the interval. Move as far left as possible.
      if (!this->move_left_if([this](const Key<T> &key) {
                          return key.start_pos() >= this->_start;
                        })) {
        break;
      }
    }

    // We're inside the interval. Recursively search the left subtree, then look
    // at this, then the right subtree.
    auto top = this->_key;
    if (this->move_left()) {
      find_first();
      if (this->_key) {
        return;
      }
    }
    this->_key = top;
    if (is_match()) {
      return;
    }
    if (this->move_right()) {
      find_first();
    } else {
      this->_key = nullptr;
    }
  }
};
