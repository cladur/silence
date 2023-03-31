#ifndef SILENCE_RETRY_NODE_H
#define SILENCE_RETRY_NODE_H

#include "../decorator_node.h"

/**
 * A retry node will retry the child node until it succeeds or fails.
 */
class RetryNode : public DecoratorNode {
protected:
	int retry_count;
	int current_retry;
public:
	RetryNode(std::string name, int retry_count) : DecoratorNode(std::move(name)), retry_count(retry_count) {}

	void enter() override {
	}

	ExecutionStatus update(float dt) override {
		ExecutionStatus status = child->update(dt);
		switch (status) {
			case ExecutionStatus::RUNNING:
				return ExecutionStatus::RUNNING;

			case ExecutionStatus::SUCCESS:
				return ExecutionStatus::SUCCESS;

			case ExecutionStatus::FAILURE:
				child->enter();
				return ExecutionStatus::RUNNING;

			case ExecutionStatus::UNDEFINED:
				return ExecutionStatus::UNDEFINED;
		}
	}
};
#endif //SILENCE_RETRY_NODE_H
