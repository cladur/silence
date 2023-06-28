#ifndef SILENCE_ENEMY_LOOKING_H
#define SILENCE_ENEMY_LOOKING_H

#include "ai/state_machine/state.h"

class EnemyLooking : public State {
private:
	bool first_frame = true;
	glm::vec3 rotation_end;
	glm::vec3 adjusted_forward;
	glm::vec3 end_forward;
	bool forward_block = false;

public:
	bool from_fully_aware = false;

	EnemyLooking() = default;
	void startup(StateMachine *machine, std::string name);
	void enter();
	void update(World *world, uint32_t entity_id, float dt);
	void exit();
};

#endif //SILENCE_ENEMY_LOOKING_H
