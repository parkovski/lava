#ifndef ASH_COLLECTIONS_TREEVECTOR_NODE_H_
#define ASH_COLLECTIONS_TREEVECTOR_NODE_H_

#include "../redblacktree/node.h"

namespace ash::collections::treevec {

template<typename Self>
struct NodeBase : rbtree::NodeBase<Self> {
  Self *set_left(Self *new_left) {
    if (_left) {
      _subtree_size -= _left->size();
    }
    if (new_left) {
      _subtree_size += new_left->size();
    }
    return rbtree::NodeBase<Self>::set_left(new_left);
  }

  Self *set_right(Self *new_right) {
    if (_right) {
      _subtree_size -= _right->size();
    }
    if (new_right) {
      _subtree_size += new_right->size();
    }
    return rbtree::NodeBase<Self>::set_right(new_right);
  }

  size_t size() const {
    return _subtree_size + 1;
  }

private:
  size_t _subtree_size = 0;
};

template<typename T, typename P, typename O>
struct Node : NodeBase<Node>, rbtree::NodeData<T> {
  using position_type = P;
  using offset_type = O;

  using NodeData::NodeData;
};

} // namespace ash::collections::treevec

#endif // ASH_COLLECTIONS_TREEVECTOR_NODE_H_
