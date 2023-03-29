#include "state_machine.h"
#include "spdlog/spdlog.h"

void StateMachine::startup() {
}

void StateMachine::update(float dt) {
	// should this be here?
	current_state->update(dt);
}

/*
 * Add a state to the state machine and assign this as its machine.
 * If the state machine is empty, set the current state to the new state.
 * If the state machine already contains the new state, do nothing.
 *
 */
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

/*
 * Set the current state to the state with the given name.
 * If the state machine does not contain a state with the given name, do nothing.
 */
void StateMachine::set_state(std::string state_name) {
	// check for existence of state with given name
	auto found = find_if (
			states.begin(),
			states.end(),
			[&state_name] (const auto& s) {
				return s->get_name() == state_name;
			} );

	if ( found != states.end() ) {
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
