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
	void add_state(State* state);
	void set_state(std::string state_name);
	void shutdown();
};

#endif //SILENCE_STATE_MACHINE_H