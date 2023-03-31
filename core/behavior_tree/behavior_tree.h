#ifndef SILENCE_BEHAVIOR_TREE_H
#define SILENCE_BEHAVIOR_TREE_H

#include "tree_node.h"
#include "leaf_node.h"
#include "decorator_node.h"
#include "composite_node.h"

class BehaviorTree {
public:

	/**
	 * Create a behavior tree with a root node.
	 */
	explicit BehaviorTree(const std::shared_ptr<TreeNode> &root) : root(root) {}

	/**
     * execute the behavior tree.
	 */
	ExecutionStatus update(float dt) {
		root->enter();
		return root->update(dt);
	}

	/**
     * recursively destroy all tree nodes.
	 */
	~BehaviorTree() {
		root->destroy();
	}

protected:
	std::shared_ptr<TreeNode> root;
};

#endif //SILENCE_BEHAVIOR_TREE_H
