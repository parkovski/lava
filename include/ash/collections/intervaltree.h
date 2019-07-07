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

  using const_outer_search_iterator = OuterSearchIterator<const T>;
  using outer_search_iterator = OuterSearchIterator<T>;

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
  // shortest.
  iterator begin() {
    return iterator(_root, _root->offset());
  }

  // Returns a const_iterator over all elements in the tree, starting with the
  // shortest.
  const_iterator begin() const {
    return const_iterator(_root, _root->offset());
  }

  // Returns a const_iterator over all elements in the tree, starting with the
  // shortest.
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
  outer_search_iterator find_outer(size_t start, size_t end) {
    return outer_search_iterator(_root, _root->offset(), start, end);
  }

  // Returns a const_iterator over all nodes that entirely overlap the range
  // [start, end).
  const_outer_search_iterator find_outer(size_t start, size_t end) const {
    return const_outer_search_iterator(_root, _root->offset(), start, end);
  }

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

  template<typename... Args>
  T &insert(size_t start, size_t end, Args &&...args) {
    return insert_node(start, end, new node_t<T>(std::forward<Args>(args)...));
  }

  void erase(const_iterator where) {
    delete extract(where);
  }

  iterator move(iterator where, size_t new_start, size_t new_end) {
    node_t<T> *node;
    // This pushes the node down to the bottom of the tree. It would be better
    // to move it up ala splay tree.
    insert_node(new_start, new_end, node = extract(where));
    return iterator(node, new_start);
  }

private:
  // Adds an interval to the tree. Sets the node's position, offset, min, max,
  // parent, and color. Does not clear left and right for pre-existing nodes.
  T &insert_node(size_t start, size_t end, node_t<T> *node) {
    size_t length = end - start;
    ptrdiff_t offset = static_cast<ptrdiff_t>(start);

    node->set_length(length);
    node->set_min_max(0, length);

    if (!_root) {
      node->set_offset(offset);
      node->set_parent(nullptr);
      node->set_color(Black);
      return (_root = node)->data();
    }

    // Find the appropriate leaf for the new node.
    node_t<T> *parent = _root;
    while (true) {
      offset -= parent->offset();

      if (length < parent->length()) {
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

    node->set_offset(offset);
    // Fix interval tracking.
    parent->update_min_max_recursive<true>();
    // Fix red-black properties.
    fix_for_insert(node);

    return node->data();
  }

  // Requires where != end. Removes the node from the tree and returns it.
  // Its parent, left, and right pointers are set to null.
  node_t<T> *extract(const_iterator where) {
    // We have a mutable reference to the tree, which has a mutable reference
    // to this node somewhere, we just don't want to go digging around trying
    // to find it.
    auto node = const_cast<node_t<T> *>(where->node());
    auto position = where->begin();
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
      auto next_position = where->begin();
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
        left->update_min_max();
      }

      auto right = node->right();
      next_node->set_right(right);
      if (right) {
        right->set_position(next_position, right->position(position));
        right->update_min_max();
      }

      // Because the removed node was above parent here, its subtree cannot
      // have gotten any smaller, but may have gotten larger. Nodes above
      // next_node may have smaller subtrees, however, but these recursive
      // calls will not overlap.
      parent->update_min_max_recursive<true>();
      next_node->update_min_max_recursive<false>();
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
        parent->update_min_max_recursive<false>();
      } else {
        _root = child;
        if (child) {
          child->set_parent(nullptr);
          child->set_offset(child->offset() + node->offset());
          child->update_min_max();
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

  // When inserting a new node, fixes the tree to maintain red-black
  // properties.
  void fix_for_insert(node_t<T> *node) {
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

  // Do the final rotation to get rid of two red nodes in a row on the same
  // side (left->left or right->right).
  void fix_for_insert_rotate(node_t<T> *node) {
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

  // Assumes node is black.
  void fix_for_erase(node_t<T> *node) {
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

  void fix_for_rotate(node_t<T> *old_pivot, node_t<T> *new_pivot,
                      node_t<T> *parent) {
    auto old_pivot_offset = old_pivot->offset();
    auto new_pivot_offset = new_pivot->offset();

    old_pivot->set_offset(-new_pivot_offset);
    new_pivot->set_offset(old_pivot_offset + new_pivot_offset);

    new_pivot->set_min_max(old_pivot->min_offset() - new_pivot_offset,
                           old_pivot->max_offset() - new_pivot_offset);
    old_pivot->update_min_max();

    if (old_pivot == _root) {
      (_root = new_pivot)->set_parent(nullptr);
    } else if (old_pivot == parent->left()) {
      parent->set_left(new_pivot);
    } else {
      parent->set_right(new_pivot);
    }
  }

  void rotate_left(node_t<T> *pivot) {
    auto new_pivot = pivot->right();
    auto parent = pivot->parent();
    pivot->set_right(new_pivot->left());
    new_pivot->set_left(pivot);

    fix_for_rotate(pivot, new_pivot, parent);
  }

  void rotate_right(node_t<T> *pivot) {
    auto new_pivot = pivot->left();
    auto parent = pivot->parent();
    pivot->set_left(new_pivot->right());
    new_pivot->set_right(pivot);

    fix_for_rotate(pivot, new_pivot, parent);
  }

  node_t<T> *_root;
};

} // namespace itree

using itree::IntervalTree;

} // namespace ash::collections

#endif // ASH_COLLECTIONS_INTERVALTREE_H_
