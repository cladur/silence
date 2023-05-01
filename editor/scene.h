#ifndef SILENCE_SCENE_H
#define SILENCE_SCENE_H

#include "camera/camera.h"
#include "ecs/ecs_manager.h"
#include "imgui.h"
#include "render/render_scene.h"

struct Scene {
	std::string name;
	bool is_visible = false;

	bool viewport_hovered = false;
	bool controlling_camera = false;
	ImVec2 last_viewport_size = ImVec2(0, 0);

	// Selection
	std::vector<Entity> entities_selected;
	Entity last_entity_selected = 0;

	Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	bool is_prefab;

	std::vector<Entity> entities;

	uint32_t render_scene_idx;

	void update(float dt);
	RenderScene &get_render_scene();
};

#endif //SILENCE_SCENE_H