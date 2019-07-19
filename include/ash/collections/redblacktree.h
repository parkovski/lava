#ifndef ASH_COLLECTIONS_REDBLACKTREE_H_
#define ASH_COLLECTIONS_REDBLACKTREE_H_

#include "redblacktree/node.h"
#include "redblacktree/iterator.h"

namespace ash::collections {
namespace rbtree {

struct RedBlackTraits {
  using position_type = size_t;
  using offset_type = ptrdiff_t;

  template<typename T>
  using node_type = Node<T, position_type, offset_type>;
};

template<typename T, typename Traits = RedBlackTraits>
class RedBlackTree {
public:
  using node_type = typename Traits::template node_type<T>;
  using position_type = typename Traits::position_type;
  using offset_type = typename Traits::offset_type;

  using iterator = Iterator<T, Traits>;
  using const_iterator = Iterator<const T, Traits>;

  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  using dfs_iterator = DFSIterator<T, Traits>;
  using const_dfs_iterator = DFSIterator<const T, Traits>;

  using reverse_dfs_iterator = std::reverse_iterator<dfs_iterator>;
  using const_reverse_dfs_iterator = std::reverse_iterator<const_dfs_iterator>;

  explicit RedBlackTree() noexcept
    : _root(nullptr)
  {}

  RedBlackTree(RedBlackTree &&other) = default;

  RedBlackTree &operator=(RedBlackTree &&other) noexcept {
    if (root()) {
      delete root();
    }
    set_root(other.root());
    other.set_root(nullptr);
    return *this;
  }

  ~RedBlackTree() {
    if (root()) {
      delete root();
    }
  }

  // Returns an iterator over all elements in the tree, starting with the
  // left-most.
  iterator begin() {
    return iterator(leftmost());
  }

  // Returns a const_iterator over all elements in the tree, starting with the
  // left-most.
  const_iterator begin() const {
    return const_iterator(leftmost());
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

  // Returns an iterator over all elements in the tree from top to bottom.
  dfs_iterator begin_dfs() {
    return dfs_iterator({root(), root()->offset()});
  }

  // Returns an iterator over all elements in the tree from top to bottom.
  const_dfs_iterator begin_dfs() const {
    return const_dfs_iterator({root(), root()->offset()});
  }

  // Returns an iterator over all elements in the tree from top to bottom.
  const_dfs_iterator cbegin_dfs() const {
    return const_dfs_iterator({root(), root()->offset()});
  }

  dfs_iterator end_dfs() {
    return dfs_iterator();
  }

  const_dfs_iterator end_dfs() const {
    return const_dfs_iterator();
  }

  const_dfs_iterator cend_dfs() const {
    return const_dfs_iterator();
  }

  // Returns an iterator over all elements in the tree, starting with the
  // right-most.
  reverse_iterator rbegin() {
    return std::make_reverse_iterator(iterator(rightmost()));
  }

  // Returns an iterator over all elements in the tree, starting with the
  // right-most.
  const_reverse_iterator rbegin() const {
    return std::make_reverse_iterator(const_iterator(rightmost()));
  }

  // Returns an iterator over all elements in the tree, starting with the
  // right-most.
  const_reverse_iterator crbegin() const {
    return std::make_reverse_iterator(const_iterator(rightmost()));
  }

  reverse_iterator rend() {
    return std::make_reverse_iterator(end());
  }

  const_reverse_iterator rend() const {
    return std::make_reverse_iterator(end());
  }

  const_reverse_iterator crend() const {
    return std::make_reverse_iterator(end());
  }

  // Returns an iterator over all elements in the tree from bottom to top.
  reverse_dfs_iterator rbegin_dfs() {
    return std::make_reverse_iterator(dfs_iterator(rightmost()));
  }

  // Returns an iterator over all elements in the tree from bottom to top.
  const_reverse_dfs_iterator rbegin_dfs() const {
    return std::make_reverse_iterator(const_dfs_iterator(rightmost()));
  }

  // Returns an iterator over all elements in the tree from bottom to top.
  const_reverse_dfs_iterator crbegin_dfs() const {
    return std::make_reverse_iterator(const_dfs_iterator(rightmost()));
  }

  reverse_dfs_iterator rend_dfs() {
    return std::make_reverse_iterator(dfs_iterator());
  }

  const_reverse_dfs_iterator rend_dfs() const {
    return std::make_reverse_iterator(const_dfs_iterator());
  }

  const_reverse_dfs_iterator crend_dfs() const {
    return std::make_reverse_iterator(const_dfs_iterator());
  }

protected:
  // Find the first node moving left from position.
  template<bool Inclusive>
  auto find_left(position_type position) {
    std::conditional_t<Inclusive, std::less<position_type>,
                       std::less_equal<position_type>> cmp{};

    Key<T, Traits> key{root(), root()->offset()};

    // Move left until we're less than (or less/equal to) position.
    while (!cmp(key.position(), position)) {
      if (!(key = key.left())) {
        return rend();
      }
    }

    // The first node is either here or in the right subtree somewhere.
    auto next = key.right();
    while (next) {
      if (cmp(next.position(), position)) {
        key = next;
        next = next.right();
      } else {
        next = next.left();
      }
    }
    return std::make_reverse_iterator(iterator(key));
  }

  // Find the first node moving right from position.
  template<bool Inclusive>
  auto find_right(position_type position) {
    std::conditional_t<Inclusive, std::greater<position_type>,
                       std::greater_equal<position_type>> cmp{};

    Key<T, Traits> key{root(), root()->offset()};

    // Move right until we're greater than (or greater/equal to) position.
    while (!cmp(key.position(), position)) {
      if (!(key = key.right())) {
        return end();
      }
    }

    // The first node is either here or in the left subtree somewhere.
    auto next = key.left();
    while (next) {
      if (cmp(next.position(), position)) {
        key = next;
        next = next.left();
      } else {
        next = next.right();
      }
    }

    return iterator(key);
  }

public:
  // Find the first node that matches position exactly.
  iterator find(position_type position) {
    auto it = find_right<true>(position);
    if (it->position() == position) {
      return it;
    }
    return end();
  }

  // Find the first node that matches position exactly.
  const_iterator find(position_type position) const {
    return const_cast<RedBlackTree *>(this)->find(position);
  }

  // Find the last node that matches position exactly.
  reverse_iterator rfind(position_type position) {
    auto it = find_left<true>(position);
    if (it->position() == position) {
      return it;
    }
    return rend();
  }

  // Find the last node that matches position exactly.
  const_reverse_iterator rfind(position_type position) const {
    return const_cast<RedBlackTree *>(this)->rfind(position);
  }

  /// Find the first node less than position. Returns a reverse iterator. To
  /// find inside ranges, compare <c>find_greater(pos)</c> against
  /// <c>find_left(pos).base()</c>.
  reverse_iterator find_less(position_type position) {
    return find_left<false>(position);
  }

  /// Find the first node less than position. Returns a reverse iterator. To
  /// find inside ranges, compare <c>find_greater(pos)</c> against
  /// <c>find_left(pos).base()</c>.
  const_reverse_iterator find_less(position_type position) const {
    return const_cast<RedBlackTree *>(this)->find_less(position);
  }

  /// Find the first node less than or equal to position. Returns a reverse
  /// iterator. To find inside ranges, compare <c>find_greater(pos)</c> against
  /// <c>find_left(pos).base()</c>.
  reverse_iterator find_less_equal(position_type position) {
    return find_left<true>(position);
  }

  /// Find the first node less than or equal to position. Returns a reverse
  /// iterator. To find inside ranges, compare <c>find_greater(pos)</c> against
  /// <c>find_left(pos).base()</c>.
  const_reverse_iterator find_less_equal(position_type position) const {
    return const_cast<RedBlackTree *>(this)->find_less_equal(position);
  }

  /// Find the first node greater than position. Returns a forward iterator.
  /// To find inside ranges, compare <c>find_greater(pos)</c> against
  /// <c>find_left(pos).base()</c>.
  iterator find_greater(position_type position) {
    return find_right<false>(position);
  }

  /// Find the first node greater than position. Returns a forward iterator.
  /// To find inside ranges, compare <c>find_greater(pos)</c> against
  /// <c>find_left(pos).base()</c>.
  const_iterator find_greater(position_type position) const {
    return const_cast<RedBlackTree *>(this)->find_greater(position);
  }

  /// Find the first node greater than position. Returns a forward iterator.
  /// To find inside ranges, compare <c>find_greater(pos)</c> against
  /// <c>find_left(pos).base()</c>.
  iterator find_greater_equal(position_type position) {
    return find_right<true>(position);
  }

  /// Find the first node greater than position. Returns a forward iterator.
  /// To find inside ranges, compare <c>find_greater(pos)</c> against
  /// <c>find_left(pos).base()</c>.
  const_iterator find_greater_equal(position_type position) const {
    return const_cast<RedBlackTree *>(this)->find_greater_equal(position);
  }

  /// Insert a node by constructing the data in place.
  /// \param pos The position of the node.
  /// \param args The arguments to pass to the data constructor.
  template<typename... Args>
  T &insert(position_type pos, Args &&...args) {
    auto node = new node_type(std::forward<Args>(args)...);
    insert_node(pos, node);
    return node->data();
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
    for (; begin != end; ++begin) {
      delete extract(begin);
    }
  }

  /// Inserts or removes space. When inserting, nodes right of \c position
  /// are shifted right. When removing, nodes between \c position and \c
  /// -space are removed from the tree, and nodes right of <c>position -
  /// space</c> are shifted left.
  /// \param position The position to insert or remove space at.
  /// \param space The amount of space to insert if positive, or to remove if
  ///        negative.
  void shift(position_type position, offset_type space);

private:
  node_type *set_root(node_type *new_root) {
    return _root = new_root;
  }

  node_type *root() {
    return _root;
  }

  const node_type *root() const {
    return _root;
  }

  Key<T, Traits> leftmost() {
    Key<T, Traits> key(_root, _root->offset());
    while (key.left()) {
      key = key.left();
    }
    return key;
  }

  Key<const T, Traits> leftmost() const {
    Key<const T, Traits> key(_root, _root->offset());
    while (key.left()) {
      key = key.left();
    }
    return key;
  }

  Key<T, Traits> rightmost() {
    Key<T, Traits> key(_root, _root->offset());
    while (key.right()) {
      key = key.right();
    }
    return key;
  }

  Key<const T, Traits> rightmost() const {
    Key<const T, Traits> key(_root, _root->offset());
    while (key.right()) {
      key = key.right();
    }
    return key;
  }

  /// Shift the position of all nodes above a certain point.
  /// \param position The start point to apply the shift.
  /// \param shift The amount to add to or subtract from the positions.
  void shift_upper(position_type position, offset_type shift);

  // Adds a node to the tree. Sets the node's position, offset, parent, and
  // color. Does not clear left and right for pre-existing nodes.
  void insert_node(position_type pos, node_type *node);

  // Requires where != end. Removes the node from the tree and returns it.
  // Its parent, left, and right pointers are set to null.
  node_type *extract(const_iterator where);

  // When inserting a new node, fixes the tree to maintain red-black
  // properties.
  void fix_for_insert(node_type *node);

  // Do the final rotation to get rid of two red nodes in a row on the same
  // side (left->left or right->right).
  void fix_for_insert_rotate(node_type *node);

  // Fix the tree to maintain red-black properties after erasing a node.
  // Assumes node is black.
  void fix_for_erase(node_type *node);

  /// Fix offsets after a rotation.
  /// \param old_pivot The pivot.
  /// \param new_pivot The node rotated into the pivot's position.
  /// \param parent The previous parent of the pivot.
  /// \param child The node reparented from \c new_pivot to \c old_pivot.
  void fix_for_rotate(node_type *old_pivot, node_type *new_pivot,
                      node_type *parent, node_type *child);

  void rotate_left(node_type *pivot);

  void rotate_right(node_type *pivot);

  node_type *_root;
};

template<typename T, typename Traits>
void RedBlackTree<T, Traits>::shift(position_type position, offset_type space) {
  if (!root() || space == 0 || position >= rightmost().position()) {
    return;
  }

  auto end = this->end();
  if (space > 0) {
    // Inserting. Shift everything right of position to the right.
    shift_upper(position + 1, space);
  } else {
    // Removing. Note space is negative in this case.
    position_type cut_end = position - space;

    // First remove inside nodes. Note the postfix increment - the order
    // of the tree is maintained, but we have to move the iterator before
    // removing the node it points to.
    //for (auto it = find_inner(position, cut_end); it != end; erase(it++)) {
    //}

    // Now move nodes to the right of the interval left.
    shift_upper(cut_end, space);
  }
}

template<typename T, typename Traits>
void RedBlackTree<T, Traits>::shift_upper(position_type position,
                                          offset_type shift) {
  position_type current_pos = 0;
  auto node = root();
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

template<typename T, typename Traits>
void RedBlackTree<T, Traits>::insert_node(position_type pos, node_type *node) {
  if (!root()) {
    node->set_offset(pos);
    node->set_parent(nullptr);
    node->set_color(Black);
    set_root(node);
    return;
  }

  // Find the appropriate leaf for the new node. Nodes are ordered by start
  // position.
  node_type *parent = root();
  position_type parent_pos = 0;
  while (true) {
    parent_pos += parent->offset();

    if (pos < parent_pos) {
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

  node->set_offset(static_cast<offset_type>(pos) - parent_pos);
  // Fix red-black properties.
  fix_for_insert(node);
}

template<typename T, typename Traits>
typename RedBlackTree<T, Traits>::node_type *
RedBlackTree<T, Traits>::extract(const_iterator where) {
  // We have a mutable reference to the tree, which has a mutable reference
  // to this node somewhere, we just don't want to go digging around trying
  // to find it.
  auto node = const_cast<node_type *>(where->node());
  auto position = where->start_pos();
  auto parent = node->parent();
  auto color = node->color();
  node_type *child;

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
    auto next_node = const_cast<node_type *>(where->node());
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
      set_root(next_node);
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
    }

    auto right = node->right();
    next_node->set_right(right);
    if (right) {
      right->set_position(next_position, right->position(position));
    }
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
    } else {
      set_root(child);
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

template<typename T, typename Traits>
void RedBlackTree<T, Traits>::fix_for_insert(node_type *node) {
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

template<typename T, typename Traits>
void RedBlackTree<T, Traits>::fix_for_insert_rotate(node_type *node) {
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

template<typename T, typename Traits>
void RedBlackTree<T, Traits>::fix_for_erase(node_type *node) {
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

template<typename T, typename Traits>
void RedBlackTree<T, Traits>::fix_for_rotate(node_type *old_pivot,
                                             node_type *new_pivot,
                                             node_type *parent,
                                             node_type *child) {
  auto old_pivot_offset = old_pivot->offset();
  auto new_pivot_offset = new_pivot->offset();

  old_pivot->set_offset(-new_pivot_offset);
  new_pivot->set_offset(old_pivot_offset + new_pivot_offset);

  if (child) {
    child->set_offset(child->offset() + new_pivot_offset);
  }

  if (!parent) {
    set_root(new_pivot)->set_parent(nullptr);
  } else if (old_pivot == parent->left()) {
    parent->set_left(new_pivot);
  } else {
    parent->set_right(new_pivot);
  }
}

template<typename T, typename Traits>
void RedBlackTree<T, Traits>::rotate_left(node_type *pivot) {
  auto new_pivot = pivot->right();
  auto parent = pivot->parent();
  auto child = pivot->set_right(new_pivot->left());
  new_pivot->set_left(pivot);

  fix_for_rotate(pivot, new_pivot, parent, child);
}

template<typename T, typename Traits>
void RedBlackTree<T, Traits>::rotate_right(node_type *pivot) {
  auto new_pivot = pivot->left();
  auto parent = pivot->parent();
  auto child = pivot->set_left(new_pivot->right());
  new_pivot->set_right(pivot);

  fix_for_rotate(pivot, new_pivot, parent, child);
}

} // namespace rbtree

using rbtree::RedBlackTree;

} // namespace ash::collections

#endif // ASH_COLLECTIONS_REDBLACKTREE_H_
