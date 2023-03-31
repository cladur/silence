#ifndef SILENCE_SUCCESS_LEAF_H
#define SILENCE_SUCCESS_LEAF_H

#include "../leaf_node.h"
#include <spdlog/spdlog.h>

/**
 * A success leaf node always returns success.
 */

class SuccessLeaf : public LeafNode {
public:
	explicit SuccessLeaf(std::string name) : LeafNode(std::move(name)) {}

	void enter() override {
		SPDLOG_INFO("entering {} leaf", name);
	}

	ExecutionStatus update(float dt) override {
		SPDLOG_INFO("{} returning success", name);
		return ExecutionStatus::SUCCESS;
	}
};

#endif //SILENCE_SUCCESS_LEAF_H
