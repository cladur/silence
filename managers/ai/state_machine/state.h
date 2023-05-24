#ifndef SILENCE_STATE_H
#define SILENCE_STATE_H

class StateMachine;
class World;

class State {
protected:
	// no idea if this should be here for now?
	// kind of no idea how it should be initialized, godot makes it look easy
	StateMachine *state_machine;

	std::string name;

public:
	State() = default;
	State(std::string name) : name(std::move(name)) {
	}
	virtual void startup(StateMachine *machine, std::string name) = 0;
	virtual void enter() = 0;
	virtual void update(World *world, uint32_t entity_id, float dt) = 0;
	virtual void exit() = 0;
	virtual std::string get_name() {
		return name;
	}
	void set_state_machine(StateMachine *machine) {
		state_machine = machine;
	};
};

#endif //SILENCE_STATE_H