#ifndef SILENCE_HACKER_SYSTEM_H
#define SILENCE_HACKER_SYSTEM_H

#include "base_system.h"
#include "components/hacker_data_component.h"
#include "components/transform_component.h"

class HackerSystem : public BaseSystem {
private:
	glm::vec3 previous_velocity;
	glm::vec3 accelerate(
			glm::vec3 accel_dir, glm::vec3 prev_velocity, float acceleration, float max_velocity, float dt);
	glm::vec3 move_ground(glm::vec3 accel_dir, glm::vec3 pre_velocity, float dt);
	bool jump_to_camera(World &world, HackerData &hacker_data, Entity camera_entity);
	bool shoot_raycast(Transform &transform, World &world, HackerData &hacker_data, float dt,
			glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f));
	void go_back_to_scorpion(World &world, HackerData &hacker_data);

	bool is_on_camera = false;
	Entity current_camera_entity = 0;

	Transform old_camera_pivot_tf;

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_HACKER_SYSTEM_H
