#ifndef ASH_COLLECTIONS_SLIDINGORDEREDSET_H_
#define ASH_COLLECTIONS_SLIDINGORDEREDSET_H_

#include "slidingorderedset/node.h"
#include "slidingorderedset/iterator.h"

namespace ash::collections {
namespace soset {

// A sorted set of numbers where all the following operations take O(log n):
// - Insert, erase at arbitrary index.
// - Get by index.
// - Get by value.
// - For some N, shift all values > N by an offset.
// Additionally, in-order traversal is constant time.
// Intended as a bidirectional mapping between line numbers and character
// positions for a text document.
// TODO: Join, erase multiple.
template<typename P = size_t, typename O = std::make_signed_t<P>,
         typename Compare = std::less<P>>
class SlidingOrderedSet {
public:
  using node_type = Node<P, O> *;
  using size_type = size_t;
  using difference_type = ptrdiff_t;
  using value_type = P;
  using offset_type = O;
  using reference = const P &;
  using const_reference = reference;
  using pointer = const P *;
  using const_pointer = pointer;

  using iterator = Iterator<P, O>;
  using const_iterator = iterator;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = reverse_iterator;

  constexpr const static size_t npos = (size_t)-1;

  explicit SlidingOrderedSet() noexcept
    : _root(nullptr)
  {}

  SlidingOrderedSet(SlidingOrderedSet &&other) = default;

  SlidingOrderedSet &operator=(SlidingOrderedSet &&other) noexcept {
    if (_root) {
      delete _root;
    }
    _root = other._root;
    other._root = nullptr;
    return *this;
  }

  ~SlidingOrderedSet() {
    if (_root) {
      delete _root;
    }
  }

  // Iterators {{{

  // Returns an iterator over all elements in the tree, starting with the
  // left-most.
  iterator begin() const {
    if (!_root) {
      return iterator();
    }
    iterator i(_root);
    i -= i._index;
    return i;
  }

  // Returns a const_iterator over all elements in the tree, starting with the
  // left-most.
  const_iterator cbegin() const {
    return begin();
  }

  // Returns a past-the-end iterator.
  iterator end() const {
    return iterator(nullptr, size(), 0);
  }

  // Returns a past-the-end const_iterator.
  const_iterator cend() const {
    return end();
  }

  // Returns an iterator over all elements in the tree, starting with the
  // right-most.
  reverse_iterator rbegin() const {
    if (!_root) {
      return std::make_reverse_iterator(iterator());
    }
    iterator i(_root);
    if (auto right = _root->right()) {
      i += right->size();
    }
    return std::make_reverse_iterator(i);
  }

  // Returns an iterator over all elements in the tree, starting with the
  // right-most.
  const_reverse_iterator crbegin() const {
    return rbegin();
  }

  reverse_iterator rend() const {
    return std::make_reverse_iterator(end());
  }

  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(end());
  }

  // Iterators }}}

  size_t size() const {
    return _root ? _root->size() : 0;
  }

  bool empty() const {
    return size() == 0;
  }

  void clear() {
    if (_root) {
      delete _root;
      _root = nullptr;
    }
  }

  /// Insert a new value into the set. Duplicates are not allowed. Returns
  /// [position, inserted].
  std::pair<iterator, bool> insert(value_type v) {
    if (!_root) {
      _root = new Node<P, O>();
      _root->set_offset(v);
      _root->set_color(Black);
      return std::make_pair(iterator(_root), true);
    }

    auto parent = insert_position(v);
    Compare less{};
    node_type node;
    size_t index;
    if (less(parent._value, v)) {
      assert(!parent._node->right());
      node = parent._node->set_right(new Node<P, O>());
      index = parent._index + 1;
    } else if (less(v, parent._value)) {
      assert(!parent._node->left());
      node = parent._node->set_left(new Node<P, O>());
      index = parent._index;
    } else {
      return std::make_pair(parent, false);
    }

    node->set_position(parent._value, v);
    fix_for_insert(node);
    parent._node->update_size();
    return std::make_pair(iterator(node, v, index), true);
  }

  /// Remove a node from the tree.
  /// \param where An iterator pointing to the node to remove.
  void erase(const_iterator where) {
    delete extract(where);
  }

  /// Remove a range of nodes from the tree.
  /// \param begin The first node to erase.
  /// \param end The first node after \c begin to keep (exclusive end of the
  ///            range).
  void erase(const_iterator begin, const_iterator end) {
    while (begin != end) {
      // Important to move the iterator before deleting the node.
      erase(begin++);
    }
  }

  // Accessors {{{

  // Get an iterator for an index.
  iterator get(size_t i) const {
    if (!_root) {
      return end();
    }

    auto node = _root;
    auto index = _root->left() ? _root->left()->size() : 0;
    auto position = _root->offset();

    while (true) {
      if (index < i) {
        if (!(node = node->right())) {
          break;
        }
        ++index;
        if (auto left = node->left()) {
          index += left->size();
        }
        position += node->offset();
      } else if (i < index) {
        if (!(node = node->left())) {
          break;
        }
        --index;
        if (auto right = node->right()) {
          index -= right->size();
        }
        position += node->offset();
      } else {
        return iterator(node, index, position);
      }
    }
    return end();
  }

  // Get an iterator for a value if it exists.
  iterator find(value_type v) const {
    if (!_root) {
      return end();
    }

    auto node = _root;
    auto index = _root->left() ? _root->left()->size() : 0;
    auto position = _root->offset();
    Compare less{};

    while (true) {
      if (less(position, v)) {
        if (!(node = node->right())) {
          break;
        }
        ++index;
        if (auto left = node->left()) {
          index += left->size();
        }
        position += node->offset();
      } else if (less(v, position)) {
        if (!(node = node->left())) {
          break;
        }
        --index;
        if (auto right = node->right()) {
          index -= right->size();
        }
        position += node->offset();
      } else {
        return iterator(node, index, position);
      }
    }
    return end();
  }

  // Returns the first node not less than (>=) v.
  iterator lower_bound(value_type v) const {
    if (!_root) {
      return end();
    }

    node_type node = nullptr;
    size_t index = 0;
    value_type position = npos;
    Compare less{};

    auto next = _root;
    size_t next_index = 0;
    if (auto left = _root->left()) {
      next_index = left->size();
    }
    value_type next_position = _root->offset();
    while (true) {
      if (less(next_position, v)) {
        if (!(next = next->right())) {
          break;
        }
        ++next_index;
        if (auto left = next->left()) {
          next_index += left->size();
        }
        next_position += next->offset();
      } else if (less(v, next_position)) {
        if (less(next_position, position)) {
          node = next;
          index = next_index;
          position = next_position;
        }
        if (!(next = next->left())) {
          break;
        }
        --next_index;
        if (auto right = next->right()) {
          next_index -= right->size();
        }
        next_position += next->offset();
      } else {
        node = next;
        index = next_index;
        position = next_position;
        break;
      }
    }
    return iterator(node, index, position);
  }

  // Returns the first node greater than v.
  iterator upper_bound(value_type v) const {
    if (!_root) {
      return end();
    }

    node_type node = nullptr;
    size_t index = 0;
    value_type position = npos;
    Compare less{};

    auto next = _root;
    size_t next_index = 0;
    if (auto left = _root->left()) {
      next_index = left->size();
    }
    value_type next_position = _root->offset();
    while (true) {
      if (!less(v, next_position)) {
        if (!(next = next->right())) {
          break;
        }
        ++next_index;
        if (auto left = next->left()) {
          next_index += left->size();
        }
        next_position += next->offset();
      } else {
        if (less(next_position, position)) {
          node = next;
          index = next_index;
          position = next_position;
        }
        if (!(next = next->left())) {
          break;
        }
        --next_index;
        if (auto right = next->right()) {
          next_index -= right->size();
        }
        next_position += next->offset();
      }
    }
    return iterator(node, index, position);
  }

  /// Returns the value at index \c i.
  value_type operator[](size_t i) const {
    return *get(i);
  }

  // Returns the index of the node pointed to by the iterator.
  size_t index_for(const const_iterator &i) const {
    if (i == end()) {
      return npos;
    }
    return i._index;
  }

  // Returns the index of the node pointed to by the iterator.
  size_t index_for(const const_reverse_iterator &i) const {
    if (i == rend()) {
      return npos;
    }
    return i.base()._index - 1;
  }

  // Accessors }}}

  /// Inserts or removes space. When inserting, nodes right of \c position
  /// are shifted right. When removing, nodes between \c position and \c
  /// -space are removed from the tree, and nodes right of <c>position -
  /// space</c> are shifted left.
  /// \param lbound The lowest value to be shifted.
  /// \param space The amount of space to insert if positive, or to remove if
  ///        negative.
  void shift(value_type lbound, offset_type offset);

private:
  /// Shift the position of all nodes above a certain point.
  /// \param position The start point to apply the shift.
  /// \param shift The amount to add to or subtract from the positions.
  void shift_upper(value_type position, offset_type shift);

  // Finds the position where a new node should be inserted.
  iterator insert_position(value_type pos);

  // Requires where != end. Removes the node from the tree and returns it.
  // Its parent, left, and right pointers are set to null.
  node_type extract(const_iterator where);

  // When inserting a new node, fixes the tree to maintain red-black
  // properties.
  void fix_for_insert(node_type node);

  // Do the final rotation to get rid of two red nodes in a row on the same
  // side (left->left or right->right).
  void fix_for_insert_rotate(node_type node);

  // Fix the tree to maintain red-black properties after erasing a node.
  // Assumes node is black.
  void fix_for_erase(node_type node);

  /// Fix offsets after a rotation.
  /// \param old_pivot The pivot.
  /// \param new_pivot The node rotated into the pivot's position.
  /// \param parent The previous parent of the pivot.
  /// \param child The node reparented from \c new_pivot to \c old_pivot.
  void fix_for_rotate(node_type old_pivot, node_type new_pivot,
                      node_type parent, node_type child);

  void rotate_left(node_type pivot);

  void rotate_right(node_type pivot);

  node_type _root;
};

template<typename P, typename O, typename Compare>
void SlidingOrderedSet<P, O, Compare>::shift(value_type position,
                                             offset_type space) {
  if (!_root || space == 0) {
    return;
  }

  if (space > 0) {
    // Inserting. Shift everything right of position to the right.
    shift_upper(position, space);
  } else {
    // Removing. Note space is negative in this case.
    value_type cut_end = position - space;

    // First remove inside nodes.
    auto first = lower_bound(position);
    auto last = lower_bound(cut_end);
    if (first != last) {
      erase(first, last);
    }

    // Now move nodes to the right of the interval left.
    shift_upper(cut_end, space);
  }
}

template<typename P, typename O, typename Compare>
void SlidingOrderedSet<P, O, Compare>::shift_upper(value_type position,
                                                   offset_type shift) {
  value_type current_pos = 0;
  auto node = _root;
  // Because node positions are represented as offsets from their parent,
  // we only need to add the shift amount to the highest nodes, and then
  // correct left children starting left of the position.
  do {
    current_pos += node->offset();
    if (current_pos < position) {
      node = node->right();
      continue;
    }

    node->set_offset(node->offset() + shift);
    // Don't update current_pos with the new offset here, as we still need
    // to correct nodes that may have crossed position but shouldn't.
    do {
      node = node->left();
      if (!node) {
        return;
      }
      current_pos += node->offset();
    } while (current_pos >= position);

    node->set_offset(node->offset() - shift);
    node = node->right();
  } while (node);
}

template<typename P, typename O, typename Compare>
typename SlidingOrderedSet<P, O, Compare>::iterator
SlidingOrderedSet<P, O, Compare>::insert_position(value_type pos) {
  if (!_root) {
    return end();
  }

  // Find the appropriate leaf for the new node. Nodes are ordered by start
  // position.
  node_type parent = _root;
  value_type parent_val = 0;
  size_t parent_index = _root->left() ? _root->left()->size() : 0;
  Compare less{};
  while (true) {
    parent_val += parent->offset();

    if (less(pos, parent_val)) {
      if (parent->left()) {
        parent = parent->left();
        --parent_index;
        if (auto right = parent->right()) {
          parent_index -= right->size();
        }
      } else {
        break;
      }
    } else if (less(parent_val, pos)) {
      if (parent->right()) {
        parent = parent->right();
        ++parent_index;
        if (auto left = parent->left()) {
          parent_index -= left->size();
        }
      } else {
        break;
      }
    } else {
      break;
    }
  }

  return iterator(parent, parent_index, parent_val);
}

template<typename P, typename O, typename Compare>
typename SlidingOrderedSet<P, O, Compare>::node_type
SlidingOrderedSet<P, O, Compare>::extract(const_iterator where) {
  auto node = where._node;
  auto value = where._value;
  auto parent = node->parent();
  auto color = node->color();
  node_type child;

  // To delete a node, we need to "move" it into a position where it has
  // no children. For a node with two children, we first move its successor
  // into its place, where the target node now has zero or one child.
  // For a node with one child, we then just move the child into its place.
  // Now the target node does not have any children and can be deleted.

  if (node->right() && node->left()) {
    // 2 children. Any node with a right child has a next node that is all
    // the way down the left subtree of that child - its left is empty.
    // Move the successor into the original node's place, and then move
    // the successor's child into the successor's place.
    ++where;
    auto next_node = where._node;
    auto next_position = where._value;
    child = next_node->right();

    if (child) {
      child->set_offset(child->offset() + next_node->offset());
    }

    auto next_parent = next_node->parent();
    if (parent) {
      if (parent->left() == node) {
        parent->set_left(next_node);
      } else {
        parent->set_right(next_node);
      }
      next_node->set_position(node->parent_position(value), next_position);
    } else {
      next_node->set_parent(nullptr);
      next_node->set_position(0, next_position);
      _root = next_node;
    }

    // It's possible that next_parent == node, so it's important that these
    // links get set before the ones on next_node below.
    if (next_node == next_parent->left()) {
      next_parent->set_left(child);
      parent = next_parent;
    } else {
      // If it's to the right, it has to be directly right of node, therefore
      // next_parent == node. The child, currently node's right, will be
      // moved under next_node.
      next_parent->set_right(child);
      parent = next_node;
    }

    next_node->set_color(color);

    auto left = node->left();
    next_node->set_left(left)
             ->set_position(next_position, left->position(value));

    auto right = node->right();
    next_node->set_right(right);
    if (right) {
      right->set_position(next_position, right->position(value));
    }
    parent->update_size();
  } else {
    // Zero or one child. Just move the child up to node's position.
    if (!(child = node->right())) {
      child = node->left();
    }
    // Move child into the place of node.
    if (parent) {
      if (node == parent->left()) {
        parent->set_left(child);
      } else {
        parent->set_right(child);
      }
      parent->update_size();
      if (child) {
        child->set_offset(child->offset() + node->offset());
      }
    } else {
      _root = child;
      if (child) {
        child->set_parent(nullptr);
        child->set_offset(child->offset() + node->offset());
      }
    }
  }

  // Now node has zero or one child (child may be null). Replace it with
  // its child and fix the tree to maintain red-black properties.
  if (child && color == Black) {
    if (child->color() == Red) {
      child->set_color(Black);
    } else if (parent) {
      fix_for_erase(child);
    }
  }
  node->unlink();
  return node;
}

template<typename P, typename O, typename Compare>
void SlidingOrderedSet<P, O, Compare>::fix_for_insert(node_type node) {
  auto parent = node->parent();

  if (!parent) {
    // Root must be black.
    node->set_color(Black);
    return;
  }

  // Assume we're inserting a red node to start with, so not to add a black
  // node to only one path.
  node->set_color(Red);

  if (parent->color() == Black) {
    // Red child of black parent: ok.
    return;
  }

  // We know there's a grandparent because parent is red, which the root
  // cannot be.
  auto grandparent = parent->parent();
  bool parent_is_left = parent == grandparent->left();
  auto uncle = parent_is_left ? grandparent->right() : grandparent->left();
  if (uncle && uncle->color() == Red) {
    // Parent, uncle, and node are red, therefore grandparent is black.
    // When a node becomes black, a higher up node must become red to
    // maintain the black node count property. But since grandparent was
    // black, its parent could have been red, which case we will have to
    // recursively fix that.
    parent->set_color(Black);
    uncle->set_color(Black);
    grandparent->set_color(Red);
    fix_for_insert(grandparent);
    return;
  }

  // Parent and node are red, but uncle and grandparent are black.
  // If node is on the inside (from grandparent, left then right or right
  // then left), we have to first rotate it to the outside of its parent so
  // that grandparent to child is a "straight line".
  // Then we can rotate around the grandparent and swap the new parent and
  // grandparent's colors to get rid of the double red.
  if (node == parent->right() && parent_is_left) {
    rotate_left(parent);
    fix_for_insert_rotate(parent);
  } else if (node == parent->left() && !parent_is_left) {
    rotate_right(parent);
    fix_for_insert_rotate(parent);
  } else {
    fix_for_insert_rotate(node);
  }
}

template<typename P, typename O, typename Compare>
void SlidingOrderedSet<P, O, Compare>::fix_for_insert_rotate(node_type node) {
  auto parent = node->parent();
  auto grandparent = parent->parent();

  if (node == parent->left()) {
    rotate_right(grandparent);
  } else {
    rotate_left(grandparent);
  }

  // At this point, grandparent is at the same height as node, and they are
  // both children of parent. Parent and node are red, grandparent is
  // black.  Swap parent and grandparent's colors to satisfy the no double
  // red property.
  parent->set_color(Black);
  grandparent->set_color(Red);
}

template<typename P, typename O, typename Compare>
void SlidingOrderedSet<P, O, Compare>::fix_for_erase(node_type node) {
  auto parent = node->parent();
  if (!parent) {
    return;
  }
  auto sibling = node->sibling();

  if (sibling->color() == Red) {
    parent->set_color(Red);
    sibling->set_color(Black);
    if (node == parent->left()) {
      rotate_left(parent);
    } else {
      rotate_right(parent);
    }
    parent = node->parent();
    sibling = node->sibling();
  }

  if (parent->color() == Black &&
      sibling->color() == Black &&
      (!sibling->left() || sibling->left()->color() == Black) &&
      (!sibling->right() || sibling->right()->color() == Black)) {
    sibling->set_color(Red);
    fix_for_erase(parent);
    return;
  }

  if (parent->color() == Red &&
      sibling->color() == Black &&
      (!sibling->left() || sibling->left()->color() == Black) &&
      (!sibling->right() || sibling->right()->color() == Black)) {
    sibling->set_color(Red);
    parent->set_color(Black);
    return;
  }

  if (sibling->color() == Black) {
    if (node == parent->left() &&
        (!sibling->right() || sibling->right()->color() == Black) &&
        (sibling->left() && sibling->left()->color() == Red)) {
      sibling->set_color(Red);
      sibling->left()->set_color(Black);
      rotate_right(sibling);
    } else if (node == parent->right() &&
               (!sibling->left() || sibling->left()->color() == Black) &&
               (sibling->right() && sibling->right()->color() == Red)) {
      sibling->set_color(Red);
      sibling->right()->set_color(Black);
      rotate_left(sibling);
    }
    parent = node->parent();
    sibling = node->sibling();
  }

  sibling->set_color(parent->color());
  parent->set_color(Black);
  if (node == parent->left()) {
    if (sibling->right()) {
      sibling->right()->set_color(Black);
    }
    rotate_left(parent);
  } else {
    if (sibling->left()) {
      sibling->left()->set_color(Black);
    }
    rotate_right(parent);
  }
}

template<typename P, typename O, typename Compare>
void SlidingOrderedSet<P, O, Compare>::fix_for_rotate(node_type old_pivot,
                                                      node_type new_pivot,
                                                      node_type parent,
                                                      node_type child) {
  auto old_pivot_offset = old_pivot->offset();
  auto new_pivot_offset = new_pivot->offset();
  auto old_pivot_size = old_pivot->size();
  auto new_pivot_size = new_pivot->size();

  old_pivot->set_offset(-new_pivot_offset);
  new_pivot->set_offset(old_pivot_offset + new_pivot_offset);
  new_pivot->set_size(old_pivot_size);
  old_pivot->set_size(old_pivot_size - new_pivot_size);

  if (child) {
    child->set_offset(child->offset() + new_pivot_offset);
  }

  if (!parent) {
    (_root = new_pivot)->set_parent(nullptr);
  } else if (old_pivot == parent->left()) {
    parent->set_left(new_pivot);
  } else {
    parent->set_right(new_pivot);
  }
}

template<typename P, typename O, typename Compare>
void SlidingOrderedSet<P, O, Compare>::rotate_left(node_type pivot) {
  auto new_pivot = pivot->right();
  auto parent = pivot->parent();
  auto child = pivot->set_right(new_pivot->left());
  new_pivot->set_left(pivot);

  fix_for_rotate(pivot, new_pivot, parent, child);
}

template<typename P, typename O, typename Compare>
void SlidingOrderedSet<P, O, Compare>::rotate_right(node_type pivot) {
  auto new_pivot = pivot->left();
  auto parent = pivot->parent();
  auto child = pivot->set_left(new_pivot->right());
  new_pivot->set_right(pivot);

  fix_for_rotate(pivot, new_pivot, parent, child);
}

} // namespace soset

using soset::SlidingOrderedSet;

} // namespace ash::collections

#endif // ASH_COLLECTIONS_SLIDINGORDEREDSET_H_
