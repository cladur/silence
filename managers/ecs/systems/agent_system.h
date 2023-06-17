#ifndef SILENCE_AGENT_SYSTEM_H
#define SILENCE_AGENT_SYSTEM_H

#include "audio/event_reference.h"
#include "base_system.h"
#include "fmod_studio.hpp"
#include "render/transparent_elements/ui/ui_elements/ui_image.h"
#include <render/transparent_elements/ui/ui_elements/ui_text.h>

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
	UIText *ui_button_hint;
	UIImage *interaction_sprite;

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
