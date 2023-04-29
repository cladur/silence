#ifndef SILENCE_SCENE_H
#define SILENCE_SCENE_H

#include "camera/camera.h"
#include "ecs/ecs_manager.h"
#include "render/render_scene.h"

struct Scene {
	std::string name;
	bool is_active = false;

	Camera camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	bool is_prefab;

	std::vector<Entity> entities;

	RenderScene *render_scene;

	void update();
};

#endif //SILENCE_SCENE_H