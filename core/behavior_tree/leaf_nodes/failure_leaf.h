#ifndef SILENCE_FAILURE_LEAF_H
#define SILENCE_FAILURE_LEAF_H

#include "../leaf_node.h"
#include <spdlog/spdlog.h>

/**
 * A failure leaf node always returns failure.
 */
class FailureLeaf : public LeafNode {
public:
	explicit FailureLeaf(std::string name) : LeafNode(std::move(name)) {}

	void enter() override {
		SPDLOG_INFO("entering {} leaf", name);
	}

	ExecutionStatus update(float dt) override {
		SPDLOG_INFO("{} returning failure", name);
		return ExecutionStatus::FAILURE;
	}
};
#endif //SILENCE_FAILURE_LEAF_H
