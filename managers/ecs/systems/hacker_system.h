#ifndef SILENCE_HACKER_SYSTEM_H
#define SILENCE_HACKER_SYSTEM_H

#include "base_system.h"

class HackerSystem : public BaseSystem {
private:
	glm::vec3 previous_velocity;
	glm::vec3 accelerate(glm::vec3 accel_dir, glm::vec3 prev_velocity, float acceleration, float max_velocity, float dt);
	glm::vec3 move_ground(glm::vec3 accel_dir, glm::vec3 pre_velocity, float dt);
public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_HACKER_SYSTEM_H
