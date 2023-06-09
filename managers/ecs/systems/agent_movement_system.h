#ifndef SILENCE_AGENT_MOVEMENT_SYSTEM_H
#define SILENCE_AGENT_MOVEMENT_SYSTEM_H

#include "base_system.h"
#include <render/transparent_elements/ui/ui_elements/ui_text.h>

class AgentMovementSystem : public BaseSystem {
private:
	glm::vec3 accelerate(
			glm::vec3 accel_dir, glm::vec3 prev_velocity, float acceleration, float max_velocity, float dt);
	glm::vec3 move_ground(glm::vec3 accel_dir, glm::vec3 pre_velocity, float acceleration, float dt);

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_AGENT_SYSTEM_H
