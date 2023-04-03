#include "state_machine.h"

void StateMachine::startup() {
}

void StateMachine::update(float dt) {
	// should this be here?
	current_state->update(dt);
}

void StateMachine::add_state(State *new_state) {
	if (states.empty()) {
		current_state = new_state;
		current_state->set_state_machine(this);
		current_state->enter();
	}
	if (std::count(states.begin(), states.end(), new_state) > 0) {
		return;
	}
	states.push_back(new_state);
}

void StateMachine::set_state(std::string state_name) {
	// check for existence of state with given name
	auto found =
			find_if(states.begin(), states.end(), [&state_name](const auto &s) { return s->get_name() == state_name; });

	if (found != states.end()) {
		current_state->exit();
		current_state = *found;
		current_state->enter();
		return;
	} else {
		SPDLOG_INFO("State machine does not contain state with name: {}", state_name);
		return;
	}
}

void StateMachine::shutdown() {
}