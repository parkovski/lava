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
    return this->_key.start_pos() < this->_end &&
           key.node()->max_pos(key.start_pos()) > this->_start;
  }

  // A match is any node that starts before the end of the search range and
  // ends after its start. We ruled out anything starting after the search
  // end already, so just compare to the start.
  bool is_match() const {
    return this->_key.start_pos() < this->_end &&
           this->_key.end_pos() > this->_start;
  }

  void find_first() {
    // Find the top-most node with an acceptable start and max.
    while (true) {
      if (this->_key.start_pos() >= this->_end) {
        if (this->move_left()) {
          continue;
        }
        this->_key = nullptr;
        return;
      }

      if (this->_key.node()->max_pos(this->_key.start_pos()) < this->_start) {
        this->_key = nullptr;
        return;
      }

      break;
    }

    // Find the left-most match.
    // TODO: This should be a BFS search, but operator++ needs to be modified.
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
