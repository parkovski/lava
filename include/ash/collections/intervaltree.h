#ifndef ASH_COLLECTIONS_INTERVALTREE_H_
#define ASH_COLLECTIONS_INTERVALTREE_H_

#include "intervaltree/node.h"
#include "intervaltree/iterator.h"
#include "intervaltree/searchiterator.h"

#include <cmath>
#include <type_traits>
#include <algorithm>
#include <functional>

namespace ash::collections {
namespace itree {

template<typename T>
class IntervalTree {
public:
  using const_iterator = Iterator<const T>;
  using iterator = Iterator<T>;

  using const_search_iterator = SearchIteratorBase<const T>;
  using search_iterator = SearchIteratorBase<T>;

  // using const_outer_search_iterator = OuterSearchIterator<const T>;
  // using outer_search_iterator = OuterSearchIterator<T>;

  using const_inner_search_iterator = InnerSearchIterator<const T>;
  using inner_search_iterator = InnerSearchIterator<T>;

  using const_overlap_search_iterator = OverlapSearchIterator<const T>;
  using overlap_search_iterator = OverlapSearchIterator<T>;

  using const_equal_search_iterator = EqualSearchIterator<const T>;
  using equal_search_iterator = EqualSearchIterator<T>;

  explicit IntervalTree() noexcept
    : _root(nullptr)
  {}

  IntervalTree(IntervalTree &&other) noexcept {
    *this = std::move(other);
  }

  IntervalTree &operator=(IntervalTree &&other) noexcept {
    _root = other._root;
    other._root = nullptr;
  }

  ~IntervalTree() {
    if (_root) {
      delete _root;
    }
  }

  // Returns an iterator over all elements in the tree, starting with the
  // left-most.
  iterator begin() {
    auto node = _root;
    size_t position = _root->offset();
    while (node->left()) {
      node = node->left();
      position += node->offset();
    }
    return iterator(node, position);
  }

  // Returns a const_iterator over all elements in the tree, starting with the
  // left-most.
  const_iterator begin() const {
    auto node = _root;
    size_t position = _root->offset();
    while (node->left()) {
      node = node->left();
      position += node->offset();
    }
    return const_iterator(node, position);
  }

  // Returns a const_iterator over all elements in the tree, starting with the
  // left-most.
  const_iterator cbegin() const {
    return begin();
  }

  // Returns a past-the-end iterator.
  iterator end() {
    return iterator();
  }

  // Returns a past-the-end const_iterator.
  const_iterator end() const {
    return const_iterator();
  }

  // Returns a past-the-end const_iterator.
  const_iterator cend() const {
    return end();
  }

  // Returns an iterator over all nodes that entirely overlap the range
  // [start, end).
  // outer_search_iterator find_outer(size_t start, size_t end) {
  //   return outer_search_iterator(_root, _root->offset(), start, end);
  // }

  // Returns a const_iterator over all nodes that entirely overlap the range
  // [start, end).
  // const_outer_search_iterator find_outer(size_t start, size_t end) const {
  //   return const_outer_search_iterator(_root, _root->offset(), start, end);
  // }

  // Returns an iterator over all nodes inside the range [start, end).
  inner_search_iterator find_inner(size_t start, size_t end) {
    return inner_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns a const_iterator over all nodes inside the range [start, end).
  const_inner_search_iterator find_inner(size_t start, size_t end) const {
    return const_inner_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns an iterator over nodes that overlap [start, end) at any point.
  overlap_search_iterator find_overlap(size_t start, size_t end) {
    return overlap_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns a const_iterator over nodes that overlap [start, end) at any
  // point.
  const_overlap_search_iterator find_overlap(size_t start, size_t end) const {
    return const_overlap_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns an iterator over nodes that contain position.
  overlap_search_iterator find(size_t position) {
    return find_overlap(position, position + 1);
  }

  // Returns an iterator over nodes that contain position.
  const_overlap_search_iterator find(size_t position) const {
    return find_overlap(position, position + 1);
  }

  // Returns an iterator over all nodes with the same range as the search
  // range.
  equal_search_iterator find_equal(size_t start, size_t end) {
    return equal_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns a const_iterator over all nodes with the same range as the search
  // range.
  const_equal_search_iterator find_equal(size_t start, size_t end) const {
    return const_equal_search_iterator(_root, _root->offset(), start, end);
  }

  /// Insert an interval by constructing the data in place.
  /// \param start The start position of the interval (inclusive).
  /// \param end The end position of the interval (exclusive).
  /// \param args The arguments to pass to the data constructor.
  template<typename... Args>
  T &insert(size_t start, size_t end, Args &&...args) {
    return insert_node(start, end, new node_t<T>(std::forward<Args>(args)...));
  }

  /// Remove an interval from the tree.
  /// \param where An iterator pointing to the interval to remove.
  void erase(const_iterator where) {
    delete extract(where);
  }

  void clear() {
    delete _root;
    _root = nullptr;
  }

  /// Shifts intervals by inserting or removing space. Overlapping intervals
  /// are expanded or shortened. Intervals entirely inside removed space are
  /// removed from the tree.
  /// \param position The position to insert or remove space at.
  /// \param space The amount of space to insert if positive, or to remove if
  ///        negative.
  void shift(size_t position, ptrdiff_t space);

private:
  /// Shift the position of all nodes above a certain point.
  /// \param position The start point to apply the shift.
  /// \param shift The amount to add to or subtract from the positions.
  void shift_upper(size_t position, ptrdiff_t shift);

  // Adds an interval to the tree. Sets the node's position, offset, max,
  // parent, and color. Does not clear left and right for pre-existing nodes.
  T &insert_node(size_t start, size_t end, node_t<T> *node);

  // Requires where != end. Removes the node from the tree and returns it.
  // Its parent, left, and right pointers are set to null.
  node_t<T> *extract(const_iterator where);

  // When inserting a new node, fixes the tree to maintain red-black
  // properties.
  void fix_for_insert(node_t<T> *node);

  // Do the final rotation to get rid of two red nodes in a row on the same
  // side (left->left or right->right).
  void fix_for_insert_rotate(node_t<T> *node);

  // Fix the tree to maintain red-black properties after erasing a node.
  // Assumes node is black.
  void fix_for_erase(node_t<T> *node);

  /// Fix offsets after a rotation.
  /// \param old_pivot The pivot.
  /// \param new_pivot The node rotated into the pivot's position.
  /// \param parent The previous parent of the pivot.
  /// \param child The node reparented from \c new_pivot to \c old_pivot.
  void fix_for_rotate(node_t<T> *old_pivot, node_t<T> *new_pivot,
                      node_t<T> *parent, node_t<T> *child);

  void rotate_left(node_t<T> *pivot);

  void rotate_right(node_t<T> *pivot);

  node_t<T> *_root;
};

template<typename T>
void IntervalTree<T>::shift(size_t position, ptrdiff_t space) {
  if (!_root || space == 0 ||
      position >= static_cast<size_t>(_root->max_pos(_root->offset()))) {
    return;
  }

  auto end = this->end();
  if (space > 0) {
    // Inserting.
    // - Nodes containing position will have their length expanded.
    // - Nodes to the right of position will have their start position
    //   shifted right.

    // First shift everything right of position to the right.
    shift_upper(position + 1, space);

    // Now find all nodes overlapping position and grow them.
    for (auto it = find(position); it != end; ++it) {
      auto node = const_cast<node_t<T> *>(it->node());
      node->set_length(it->length() + space);
      node->template update_max_recursive<true>();
    }
  } else {
    // Removing. Note space is negative in this case.
    // - Nodes inside [position, position - space) are removed.
    // - Nodes overlapping the interval on the left are shortened.
    // - Nodes overlapping the interval only on the right are shortened and
    //   are also shifted left.
    // - Nodes to the right of that interval are shifted left.
    size_t cut_end = position - space;

    // First remove inside nodes. Note the postfix increment - the order
    // of the tree is maintained, but we have to move the iterator before
    // removing the node it points to.
    for (auto it = find_inner(position, cut_end); it != end; erase(it++)) {
    }

    // Shorten the overlapping nodes and move nodes starting inside the
    // removal interval to its start. After this loop is complete, nothing
    // will be left that starts within the removal interval, so even though
    // some nodes may move by different distances, all nodes on their left
    // either start before the interval and are unaffected, or are also moved
    // left to the start of the interval.
    for (auto it = find_overlap(position, cut_end); it != end; ++it) {
      auto node = const_cast<node_t<T> *>(it->node());
      auto length = node->length();

      if (it->start_pos() > position) {
        // Right overlap only.
        length -= cut_end - it->start_pos();
        node->set_length(length);

        // Adjust the offset and fix the child offsets.
        auto start_ofs = it->start_pos() - position;
        node->set_offset(node->offset() - start_ofs);
        if (auto left = node->left(); left) {
          left->set_offset(left->offset() + start_ofs);
        }
        if (auto right = node->right(); right) {
          right->set_offset(right->offset() + start_ofs);
        }

        node->template update_max_recursive<false>();
        continue;
      }

      if (it->end_pos() < cut_end) {
        // Left overlap only.
        length -= it->end_pos() - position;
      } else {
        // Both overlap.
        length += space;
      }
      node->set_length(length);
      node->template update_max_recursive<false>();
    }

    // Now the only thing left is to move nodes to the right of the interval
    // left by the interval distance.
    shift_upper(cut_end, space);
  }
}

template<typename T>
void IntervalTree<T>::shift_upper(size_t position, ptrdiff_t shift) {
  size_t current_pos = 0;
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
    // Also don't update the max, because it is relative to self position.
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

template<typename T>
T &IntervalTree<T>::insert_node(size_t start, size_t end, node_t<T> *node) {
  size_t length = end - start;

  node->set_length(length);
  node->set_max_offset(length);

  if (!_root) {
    node->set_offset(start);
    node->set_parent(nullptr);
    node->set_color(Black);
    return (_root = node)->data();
  }

  // Find the appropriate leaf for the new node. Nodes are ordered by start
  // position.
  node_t<T> *parent = _root;
  size_t position = 0;
  while (true) {
    position += parent->offset();

    if (start < position) {
      if (parent->left()) {
        parent = parent->left();
        continue;
      }
      parent->set_left(node);
      break;
    } else {
      if (parent->right()) {
        parent = parent->right();
        continue;
      }
      parent->set_right(node);
      break;
    }
  }

  node->set_offset(static_cast<ptrdiff_t>(start) - position);
  // Fix interval tracking.
  parent->template update_max_recursive<true>();
  // Fix red-black properties.
  fix_for_insert(node);

  return node->data();
}

template<typename T>
node_t<T> *IntervalTree<T>::extract(IntervalTree<T>::const_iterator where) {
  // We have a mutable reference to the tree, which has a mutable reference
  // to this node somewhere, we just don't want to go digging around trying
  // to find it.
  auto node = const_cast<node_t<T> *>(where->node());
  auto position = where->start_pos();
  auto parent = node->parent();
  auto color = node->color();
  node_t<T> *child;

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
    auto next_node = const_cast<node_t<T> *>(where->node());
    auto next_position = where->start_pos();
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
      next_node->set_position(node->parent_position(position), next_position);
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
    next_node->set_left(left);
    if (left) {
      left->set_position(next_position, left->position(position));
      left->update_max();
    }

    auto right = node->right();
    next_node->set_right(right);
    if (right) {
      right->set_position(next_position, right->position(position));
      right->update_max();
    }

    // Because the removed node was above parent here, its subtree cannot
    // have gotten any smaller, but may have gotten larger. Nodes above
    // next_node may have smaller subtrees, however, but these recursive
    // calls will not overlap.
    parent->template update_max_recursive<true>();
    next_node->template update_max_recursive<false>();
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
      if (child) {
        child->set_offset(child->offset() + node->offset());
      }
      // Because a node was removed, the subtree length may now be smaller.
      parent->template update_max_recursive<false>();
    } else {
      _root = child;
      if (child) {
        child->set_parent(nullptr);
        child->set_offset(child->offset() + node->offset());
        child->update_max();
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

template<typename T>
void IntervalTree<T>::fix_for_insert(node_t<T> *node) {
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

template<typename T>
void IntervalTree<T>::fix_for_insert_rotate(node_t<T> *node) {
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

template<typename T>
void IntervalTree<T>::fix_for_erase(node_t<T> *node) {
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

template<typename T>
void IntervalTree<T>::fix_for_rotate(node_t<T> *old_pivot,
                                     node_t<T> *new_pivot,
                                     node_t<T> *parent,
                                     node_t<T> *child) {
  auto old_pivot_offset = old_pivot->offset();
  auto new_pivot_offset = new_pivot->offset();

  old_pivot->set_offset(-new_pivot_offset);
  new_pivot->set_offset(old_pivot_offset + new_pivot_offset);

  if (child) {
    child->set_offset(child->offset() + new_pivot_offset);
  }

  new_pivot->set_max_offset(old_pivot->max_offset() - new_pivot_offset);
  old_pivot->update_max();

  if (!parent) {
    (_root = new_pivot)->set_parent(nullptr);
  } else if (old_pivot == parent->left()) {
    parent->set_left(new_pivot);
  } else {
    parent->set_right(new_pivot);
  }
}

template<typename T>
void IntervalTree<T>::rotate_left(node_t<T> *pivot) {
  auto new_pivot = pivot->right();
  auto parent = pivot->parent();
  auto child = pivot->set_right(new_pivot->left());
  new_pivot->set_left(pivot);

  fix_for_rotate(pivot, new_pivot, parent, child);
}

template<typename T>
void IntervalTree<T>::rotate_right(node_t<T> *pivot) {
  auto new_pivot = pivot->left();
  auto parent = pivot->parent();
  auto child = pivot->set_left(new_pivot->right());
  new_pivot->set_right(pivot);

  fix_for_rotate(pivot, new_pivot, parent, child);
}

} // namespace itree

using itree::IntervalTree;

} // namespace ash::collections

#endif // ASH_COLLECTIONS_INTERVALTREE_H_
