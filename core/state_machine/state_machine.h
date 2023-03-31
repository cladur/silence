#ifndef SILENCE_STATE_MACHINE_H
#define SILENCE_STATE_MACHINE_H

#include "states/state.h"
#include <vector>
class StateMachine {
private:
	State* current_state;
	std::vector<State*> states;

public:
	void startup();
	void update(float dt);

	/*
	* Add a state to the state machine and assign this as its machine.
	* If the state machine is empty, set the current state to the new state.
	* If the state machine already contains the new state, do nothing.
	*/
	void add_state(State* state);

	/*
	 * Set the current state to the state with the given name.
	 * If the state machine does not contain a state with the given name, do nothing.
	 */
	void set_state(std::string state_name);
	void shutdown();
};

#endif //SILENCE_STATE_MACHINE_H