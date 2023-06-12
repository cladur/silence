#ifndef SILENCE_HACKER_SYSTEM_H
#define SILENCE_HACKER_SYSTEM_H

#include "base_system.h"
#include "components/hacker_data_component.h"
#include "components/transform_component.h"
#include <render/transparent_elements/ui/ui_elements/ui_text.h>

class HackerSystem : public BaseSystem {
private:
	glm::vec3 previous_velocity;
	bool jump_to_camera(World &world, HackerData &hacker_data, Entity camera_entity);
	bool shoot_raycast(Transform &transform, World &world, HackerData &hacker_data, float dt, bool trigger,
			glm::vec3 direction = glm::vec3(0.0f, 0.0f, -1.0f));
	void go_back_to_scorpion(World &world, HackerData &hacker_data);

	float current_rotation_x = 0.0f;
	float current_rotation_y = 0.0f;

	glm::quat starting_camera_pivot_orientation;
	glm::quat starting_camera_orientation;

	float current_rotation_x_camera_pivot = 0.0f;
	float current_rotation_x_camera = 0.0f;

	std::string ui_name;
	UIText *ui_text;

	bool is_on_camera = false;
	Entity current_camera_entity = 0;

	float default_fov;
	bool first_frame = true;
	float camera_sens_modifier = 1.0f;
	bool is_zooming = false;

	Transform old_camera_pivot_tf;

public:
	void startup(World &world) override;
	void update(World &world, float dt) override;
};

#endif //SILENCE_HACKER_SYSTEM_H
