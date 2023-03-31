#ifndef SILENCE_SEQUENCE_NODE_H
#define SILENCE_SEQUENCE_NODE_H

#include "../composite_node.h"

/**
 * A sequence node is a node that has multiple children.
 * The sequence node will execute all children in order.
 * If a child fails, the sequence node will fail.
 * If all children succeed, the sequence node will succeed.
 */

class SequenceNode : public CompositeNode {
protected:
	int current_running_child = -1;
public:
	explicit SequenceNode(std::string name) : CompositeNode(std::move(name)) {
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
		if (status == ExecutionStatus::RUNNING) {
			return ExecutionStatus::RUNNING;
		}

		if (status == ExecutionStatus::FAILURE) {
			SPDLOG_INFO("sequence node failed, index reset, returning fail.");
			current_running_child = -1;
			return ExecutionStatus::FAILURE;
		}

		if (status == ExecutionStatus::SUCCESS) {
			current_running_child++;
			if (current_running_child == children.size()) {
				SPDLOG_INFO("all children succeeded, returning success.");
				current_running_child = -1;
				return ExecutionStatus::SUCCESS;
			}
			children[current_running_child]->enter();
			return ExecutionStatus::RUNNING;
		}

		// this should never happen tho
		return ExecutionStatus::UNDEFINED;
	}
};
#endif //SILENCE_SEQUENCE_NODE_H
