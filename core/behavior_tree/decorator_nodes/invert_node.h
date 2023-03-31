#ifndef SILENCE_INVERT_NODE_H
#define SILENCE_INVERT_NODE_H

#include "behavior_tree/decorator_node.h"

/**
 * An invert node will invert the result of the child node.
 */
class InvertNode : public DecoratorNode {
public:
	explicit InvertNode(std::string name) : DecoratorNode(std::move(name)) {
	}

	void enter() override {
	}

	ExecutionStatus update(float dt) override {
		ExecutionStatus status = child->update(dt);
		switch (status) {
			case ExecutionStatus::RUNNING:
				return ExecutionStatus::RUNNING;

			case ExecutionStatus::SUCCESS:
				return ExecutionStatus::FAILURE;

			case ExecutionStatus::FAILURE:
				return ExecutionStatus::SUCCESS;

			case ExecutionStatus::UNDEFINED:
				return ExecutionStatus::UNDEFINED;
		}
	}
};
#endif //SILENCE_INVERT_NODE_H
