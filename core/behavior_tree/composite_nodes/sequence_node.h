#ifndef SILENCE_SEQUENCE_NODE_H
#define SILENCE_SEQUENCE_NODE_H

#include "behavior_tree/composite_node.h"

/**
 * A sequence node is a node that has multiple children.
 * The sequence node will execute each of the children until it returns anything other than RUNNING.
 * If a child returns FAILURE, the sequence node will return FAILURE.
 * If a child returns SUCCESS, the sequence node will execute the next child, or return SUCCESS if there are no more
 * children.
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
