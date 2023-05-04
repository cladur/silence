#ifndef SILENCE_SCENE_H
#define SILENCE_SCENE_H

#include "camera/camera.h"
#include "ecs/world.h"
#include "imgui.h"
#include "render/render_scene.h"

struct Scene {
	std::string name;
	std::string path;
	bool is_prefab;

	World world;

	Camera camera;

	std::vector<Entity> entities;

	uint32_t render_scene_idx;

	Scene();
	virtual void update(float dt);
	RenderScene &get_render_scene();

	// Serialization
	virtual void save_to_file(const std::string &path);
	void load_from_file(const std::string &path);
};

#endif //SILENCE_SCENE_H