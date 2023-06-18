#ifndef SILENCE_ENEMY_DISTRACTED_H
#define SILENCE_ENEMY_DISTRACTED_H

#include <ai/state_machine/state.h>

class EnemyDistracted : public State {
	bool first_frame = true;
	bool cooldown_extended = false;
public:
	EnemyDistracted() = default;
	void startup(StateMachine *machine, std::string name);
	void enter();
	void update(World *world, uint32_t entity_id, float dt);
	void exit();
};

#endif //SILENCE_ENEMY_DISTRACTED_H
