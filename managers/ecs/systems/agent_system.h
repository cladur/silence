#ifndef SILENCE_AGENT_SYSTEM_H
#define SILENCE_AGENT_SYSTEM_H

#include "base_system.h"
#include <render/transparent_elements/ui/ui_elements/ui_text.h>
#include "fmod_studio.hpp"
#include "audio/event_reference.h"

class AgentSystem : public BaseSystem {
private:
	float animation_timer;
	float default_fov;
	float current_rotation_y_camera_pivot = 0.0f;
	bool first_frame = true;
	float camera_sens_modifier = 1.0f;
	bool is_zooming = false;

	std::string ui_name;
	UIText *ui_interaction_text;
	glm::vec3 default_interaction_text_position = glm::vec3(150.0f, 3.0f, 0.0f);
	UIText *ui_kill_text;

	float current_step_time = 0.0f;
	bool first_step = true;
	float first_step_offset = 0.125f;
	float time_per_step = 0.30555; // calculated as 21 * (1 / 72). Thats what i counted from looking at a recording of our animation

	EventReference jump_event;
	EventReference stab_event;

	EventReference footsteps_event;
	std::string left_foot_bone = "heel.02.L";
	std::string right_foot_bone = "heel.02.R";
	float footstep_left_down_threshold = 0.007f;
	float footstep_right_down_threshold = 0.009f;
	float footstep_up_threshold = 0.1f;
	bool left_foot_can_play = false;
	bool right_foot_can_play = false;

public:
	~AgentSystem();
	void startup(World &world) override;
	void update(World &world, float dt) override;

	void reset();
};

#endif //SILENCE_AGENT_SYSTEM_H
