#ifndef SILENCE_LEAF_NODE_H
#define SILENCE_LEAF_NODE_H

#include "tree_node.h"

/**
 * A leaf node is a node that has no children.
 */
class LeafNode : public TreeNode {
public:
	explicit LeafNode(std::string name) : TreeNode(std::move(name)) {}
};

#endif //SILENCE_LEAF_NODE_H
