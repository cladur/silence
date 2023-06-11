#ifndef SILENCE_AGENT_SYSTEM_H
#define SILENCE_AGENT_SYSTEM_H

#include "base_system.h"
#include <render/transparent_elements/ui/ui_elements/ui_text.h>

class AgentSystem : public BaseSystem {
private:
	float animation_timer;
	bool is_crouching;
	bool is_climbing;
	glm::vec3 previous_velocity;
	float default_fov;
	bool first_frame = true;
	float camera_sens_modifier = 1.0f;
	bool is_zooming = false;

	std::string ui_name;
	UIText *ui_interaction_text;
	UIText *ui_kill_text;

	glm::vec3 accelerate(
			glm::vec3 accel_dir, glm::vec3 prev_velocity, float acceleration, float max_velocity, float dt);
	glm::vec3 move_ground(glm::vec3 accel_dir, glm::vec3 pre_velocity, float dt);

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_AGENT_SYSTEM_H
