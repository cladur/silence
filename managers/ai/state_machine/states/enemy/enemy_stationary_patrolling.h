#ifndef SILENCE_ENEMY_STATIONARY_PATROLLING_H
#define SILENCE_ENEMY_STATIONARY_PATROLLING_H

#include "ai/state_machine/state.h"

class EnemyStationaryPatrolling : public State {
public:
	EnemyStationaryPatrolling() = default;
	void startup(StateMachine *machine, std::string name);
	void enter();
	void update(World *world, uint32_t entity_id, float dt);
	void exit();
};


#endif //SILENCE_ENEMY_STATIONARY_PATROLLING_H
