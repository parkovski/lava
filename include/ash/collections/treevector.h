#ifndef ASH_COLLECTIONS_TREEVECTOR_H_
#define ASH_COLLECTIONS_TREEVECTOR_H_

#include "redblacktree.h"
#include "treevector/node.h"

namespace ash::collections {
namespace treevec {

template<typename P = size_t, typename O = std::make_signed_t<P>>
struct TreeVectorTraits {
  using position_type = P;
  using offset_type = O;

  template<typename T>
  using node_type = Node<T, position_type, offset_type>;
};

template<typename T, typename Traits = TreeVectorTraits<>>
class TreeVector : public RedBlackTree<T, Traits> {
  using base = RedBlackTree<T, Traits>;

  using typename base::node_type;
  using typename base::position_type;
  using typename base::offset_type;

  using typename base::iterator;
  using typename base::const_iterator;

  using typename base::reverse_iterator;
  using typename base::const_reverse_iterator;

  using typename base::dfs_iterator;
  using typename base::const_dfs_iterator;

  using typename base::reverse_dfs_iterator;
  using typename base::const_reverse_dfs_iterator;

  using base::RedBlackTree;

  size_t size() const {
    if (!root()) {
      return 0;
    }
    return root()->size();
  }

  iterator get(size_t index) {
    Key<T, Traits> key{root(), root()->offset()};
    auto key_index = 0;
    if (key.node()->left()) {
      key_index += key.node()->left()->size();
    }
    while (key_index != index) {
      if (index < key_index) {
        --key_index;
        key = key.left();
        if (!key) {
          return end();
        }
        if (key.node()->right()) {
          key_index -= key.node()->right()->size();
        }
      } else {
        ++key_index;
        key = key.right();
        if (!key) {
          return end();
        }
        if (key.node()->left()) {
          key_index += key.node()->left()->size();
        }
      }
    }
  }

  const_iterator get(size_t index) const {
    return const_cast<TreeVector *>(this)->get(index);
  }

  T &operator[](size_t index) {
    auto node = root();
    auto node_index = 0;
    if (node->left()) {
      node_index += node->left()->size();
    }
    while (key_index != index) {
      if (index < node_index) {
        --node_index;
        node = node->left();
        if (node->right()) {
          node_index -= node->right()->size();
        }
      } else {
        ++node_index;
        node = node->right();
        if (node->left()) {
          node_index += node->left()->size();
        }
      }
    }
    return node->data();
  }

  const T &operator[](size_t index) const {
    return const_cast<TreeVector *>(this)->operator[](index);
  }
};

} // namespace treevec



} // namespace ash::collections

#endif // ASH_COLLECTIONS_TREEVECTOR_H_
