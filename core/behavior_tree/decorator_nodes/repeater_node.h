#ifndef SILENCE_REPEATER_NODE_H
#define SILENCE_REPEATER_NODE_H

#include "../decorator_node.h"

/**
 * A repeater node is a decorator node that repeats the execution of its child node a certain amount of times.
 * If the child node returns success, the repeater node will run the child node again until the repeat count is reached.
 */
class RepeaterNode : public DecoratorNode {
protected:
	int repeat_count;
	int current_repeat;
public:
	RepeaterNode(std::string name, int repeat_count) : DecoratorNode(std::move(name)), repeat_count(repeat_count) {
		current_repeat = 0;
	}

	void enter() override {
		SPDLOG_INFO("entering repeater node, repeat count: {}", repeat_count);
	}

	ExecutionStatus update(float dt) override {
		ExecutionStatus status = child->update(dt);
		switch (status) {
				// if the child is running, we return running
			case ExecutionStatus::RUNNING:
				return ExecutionStatus::RUNNING;

				// if the child succeeds, we increment the repeat count and either return success or run the child again
			case ExecutionStatus::SUCCESS:
				current_repeat++;
				if (current_repeat < repeat_count) {
					child->enter();
					return ExecutionStatus::RUNNING;
				} else {
					SPDLOG_INFO("repeater node succeeded, index reset, returning success.");
					current_repeat = 0;
					return ExecutionStatus::SUCCESS;
				}

			case ExecutionStatus::FAILURE:
				return ExecutionStatus::FAILURE;
		}
	}
};

#endif //SILENCE_REPEATER_NODE_H
