#ifndef SILENCE_AGENT_SYSTEM_H
#define SILENCE_AGENT_SYSTEM_H

#include "base_system.h"
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
	UIText *ui_kill_text;

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_AGENT_SYSTEM_H
