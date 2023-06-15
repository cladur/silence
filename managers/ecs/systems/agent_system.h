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
	glm::vec3 default_interaction_text_position = glm::vec3(150.0f, 3.0f, 0.0f);
	UIText *ui_kill_text;

public:
	~AgentSystem();
	void startup(World &world) override;
	void update(World &world, float dt) override;

	void reset();
};

#endif //SILENCE_AGENT_SYSTEM_H
