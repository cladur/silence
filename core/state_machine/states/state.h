#ifndef SILENCE_STATE_H
#define SILENCE_STATE_H

//#include "state_machine.h"
#include <string>
#include <utility>

class StateMachine;

class State {
private:
	// no idea if this should be here for now?
	// kind of no idea how it should be initialized, godot makes it look easy
	StateMachine *state_machine;

	std::string name;

public:
	State(std::string name) : name(std::move(name)) {}
	virtual void enter() = 0;
	virtual void update(float dt) = 0;
	virtual void exit() = 0;
	virtual std::string get_name() { return name; }
	void set_state_machine(StateMachine *machine) {
		state_machine = machine;
	};
};

#endif //SILENCE_STATE_H