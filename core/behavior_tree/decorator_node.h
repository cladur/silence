#ifndef SILENCE_DECORATOR_NODE_H
#define SILENCE_DECORATOR_NODE_H

#include "tree_node.h"

/**
 * A decorator node is a node that has one child.
 */
class DecoratorNode : public TreeNode {
protected:
	std::shared_ptr<TreeNode> child;

public:
	explicit DecoratorNode(std::string name) : TreeNode(std::move(name)) {}

	/**
	 * Add a child to the decorator node.
	 */
	void set_child(std::shared_ptr<TreeNode> child) {
		this->child = child;
	}

	/**
	 * Remove a child from the decorator node.
	 */
	void remove_child(std::shared_ptr<TreeNode> child) {
		this->child = nullptr;
		child->set_parent(nullptr);
	}

	void destroy() override {
		child->destroy();
		TreeNode::destroy();
	}
};
#endif //SILENCE_DECORATOR_NODE_H
