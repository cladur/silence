#ifndef SILENCE_ENEMY_FULLY_AWARE_H
#define SILENCE_ENEMY_FULLY_AWARE_H

#include "ai/state_machine/state.h"

class EnemyFullyAware :public State {
public:
	EnemyFullyAware() = default;
	void startup(StateMachine *machine, std::string name);
	void enter();
	void update(World *world, uint32_t entity_id, float dt);
	void exit();
};

#endif //SILENCE_ENEMY_FULLY_AWARE_H
