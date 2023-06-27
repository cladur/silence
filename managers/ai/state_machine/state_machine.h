#ifndef SILENCE_STATE_MACHINE_H
#define SILENCE_STATE_MACHINE_H

class State;
class World;

class StateMachine {
private:
	State *current_state;
	std::vector<State *> states;

public:
	StateMachine() = default;
	void startup();
	void update(World *world, uint32_t entity_id, float dt);

	/*
	 * Add a state to the state machine and assign this as its machine.
	 * If the state machine is empty, set the current state to the new state.
	 * If the state machine already contains the new state, do nothing.
	 */
	void add_state(State *state);

	/*
	 * Set the current state to the state with the given name.
	 * If the state machine does not contain a state with the given name, do nothing.
	 */
	void set_state(std::string state_name);
	void shutdown();

	template<typename T> T *get_state() {
		for (auto state : states) {
			if (dynamic_cast<T *>(state) != nullptr) {
				return dynamic_cast<T *>(state);
			}
		}
		return nullptr;
	}

	std::string get_current_state();
};

#endif //SILENCE_STATE_MACHINE_H