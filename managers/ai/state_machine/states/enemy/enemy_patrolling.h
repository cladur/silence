#ifndef SILENCE_ENEMY_PATROLLING_H
#define SILENCE_ENEMY_PATROLLING_H

#include "ai/state_machine/state.h"

class EnemyPatrolling : public State {
public:
	bool first_frame_after_other_state = false;
	EnemyPatrolling() = default;
	void startup(StateMachine *machine, std::string name);
	void enter();
	void update(World *world, uint32_t entity_id, float dt);
	void exit();
};

#endif //SILENCE_ENEMY_PATROLLING_H
