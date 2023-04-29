#include "render_system.h"

#include "core/components/transform_component.h"
#include "ecs/ecs_manager.h"
#include "managers/render/render_manager.h"

extern ECSManager ecs_manager;

void RenderSystem::startup() {
	Signature signature;
	signature.set(ecs_manager.get_component_type<Transform>());
	signature.set(ecs_manager.get_component_type<ModelInstance>());
	ecs_manager.set_system_component_whitelist<RenderSystem>(signature);
}

void RenderSystem::update() {
	RenderManager &render_manager = *RenderManager::get();

	for (auto const &entity : entities) {
		auto &transform = ecs_manager.get_component<Transform>(entity);
		auto &model_instance = ecs_manager.get_component<ModelInstance>(entity);

		render_manager.queue_draw(&model_instance, &transform);
	}
}