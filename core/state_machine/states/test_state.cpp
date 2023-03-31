#include "test_state.h"

#include <utility>
#include "spdlog/spdlog.h"

TestState::TestState(std::string name) : State(std::move(name)) {
}

void TestState::enter() {
	std::string print_name = get_name();
	SPDLOG_INFO("entering state {}", print_name);
}

void TestState::update(float dt) {
}

void TestState::exit() {
}