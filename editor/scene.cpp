#include "scene.h"
#include "ecs/ecs_manager.h"
#include "render/ecs/model_instance.h"

void Scene::update() {
	ECSManager &ecs_manager = ECSManager::get();

	render_scene->camera = camera;

	for (auto &entity : entities) {
		if (ecs_manager.has_component<Transform>(entity) && ecs_manager.has_component<ModelInstance>(entity)) {
			auto &transform = ecs_manager.get_component<Transform>(entity);
			auto &model_instance = ecs_manager.get_component<ModelInstance>(entity);

			render_scene->queue_draw(&model_instance, &transform);
		}
	}
}
