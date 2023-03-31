#ifndef SILENCE_COMPOSITE_NODE_H
#define SILENCE_COMPOSITE_NODE_H

#include "tree_node.h"
#include <vector>

/**
 * A composite node is a node that has two or more children.
 */
class CompositeNode : public TreeNode {
protected:
	std::vector<std::shared_ptr<TreeNode>> children;

public:
	explicit CompositeNode(std::string name) : TreeNode(std::move(name)) {}

	/**
	 * Add a child to the composite node.
	 */
	void add_child(std::shared_ptr<TreeNode> child) {
		this->children.emplace_back(child);

	}

	/**
	 * Remove a child from the composite node.
	 */
	void remove_child(std::shared_ptr<TreeNode> child) {
		children.erase(std::remove(children.begin(), children.end(), child), children.end());
		child->set_parent(nullptr);
	}

	void destroy() override {
		for (auto &child : children) {
			child->destroy();
		}
		TreeNode::destroy();
	}

};

#endif //SILENCE_COMPOSITE_NODE_H
