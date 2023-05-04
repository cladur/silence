#include "render_system.h"

#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "managers/render/render_manager.h"

void RenderSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Transform>());
	signature.set(world.get_component_type<ModelInstance>());
	world.set_system_component_whitelist<RenderSystem>(signature);
}

void RenderSystem::update(World &world, float dt) {
	RenderManager &render_manager = RenderManager::get();

	// for (auto const &entity : entities) {
	// 	auto &transform = world.get_component<Transform>(entity);
	// 	auto &model_instance = world.get_component<ModelInstance>(entity);

	// 	render_manager.queue_draw(&model_instance, &transform);
	// }
}