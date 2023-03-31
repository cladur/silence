#ifndef SILENCE_SELECTOR_NODE_H
#define SILENCE_SELECTOR_NODE_H

#include "behavior_tree/composite_node.h"

/**
 * A selector node is a composite node that will execute its children in order until one of them succeeds.
 * If all children fail, the selector node will fail.
 */
class SelectorNode : public CompositeNode {
protected:
	int current_running_child = -1;

public:
	explicit SelectorNode(std::string name) : CompositeNode(std::move(name)) {
		current_running_child = -1;
	}

	void enter() override {
		current_running_child = -1;
	}

	ExecutionStatus update(float dt) override {
		if (children.empty()) {
			return ExecutionStatus::SUCCESS;
		}

		if (current_running_child == -1) {
			current_running_child = 0;
			children[current_running_child]->enter();
		}

		ExecutionStatus status = children[current_running_child]->update(dt);
		switch (status) {
			// if a child is running, return running
			case ExecutionStatus::RUNNING:
				return ExecutionStatus::RUNNING;

			// if a child fails, try the next child, if there is none, return failure
			case ExecutionStatus::FAILURE:
				current_running_child++;
				if (current_running_child == children.size()) {
					SPDLOG_INFO("all children failed, returning failure.");
					current_running_child = -1;
					return ExecutionStatus::FAILURE;
				}
				children[current_running_child]->enter();
				return ExecutionStatus::RUNNING;

			// if a child succeeds, return success
			case ExecutionStatus::SUCCESS:
				SPDLOG_INFO("selector node succeeded, index reset, returning success.");
				current_running_child = -1;
				return ExecutionStatus::SUCCESS;
			default:
				break;
		}

		// this should never happen tho
		return ExecutionStatus::UNDEFINED;
	}
};

#endif //SILENCE_SELECTOR_NODE_H
