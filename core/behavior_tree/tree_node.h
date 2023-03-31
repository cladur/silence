#ifndef SILENCE_TREE_NODE_H
#define SILENCE_TREE_NODE_H

enum class ExecutionStatus {
	SUCCESS,
	FAILURE,
	RUNNING,
	UNDEFINED,
};

class TreeNode {
protected:
	std::string name;
	std::shared_ptr<TreeNode> parent;

public:
	// constructor
	explicit TreeNode(std::string name) : name(std::move(name)) {
	}

	virtual void enter() = 0;

	/**
	 * Process the current node.
	 * Will return SUCCESS if the node has finished successfully.
	 * Will return FAILURE if the node has finished unsuccessfully.
	 * Will return RUNNING if the node is still running.
	 */
	virtual ExecutionStatus update(float dt) = 0;

	std::string get_name() {
		return name;
	}

	std::shared_ptr<TreeNode> get_parent() {
		return parent;
	}
	void set_parent(std::shared_ptr<TreeNode> new_parent) {
		this->parent = new_parent;
	}

	virtual void destroy() {
		this->parent = nullptr;
	};
};

#endif //SILENCE_TREE_NODE_H
