#include "render_system.h"

#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include "managers/render/render_manager.h"

void RenderSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Transform>());
	signature.set(world.get_component_type<ModelInstance>());
	world.set_system_component_whitelist<RenderSystem>(signature);
}

void RenderSystem::update(World &world, float dt) {
	ZoneScopedN("RenderSystem::update");
	RenderManager &render_manager = RenderManager::get();
	for (auto const &entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &model_instance = world.get_component<ModelInstance>(entity);

		HighlightData highlight = {};

		if (world.has_component<Highlight>(entity)) {
			auto &hc = world.get_component<Highlight>(entity);
			highlight.highlighted = hc.highlighted;
			highlight.highlight_color = hc.highlight_color;
			highlight.target = hc.target;
			highlight.highlight_power = hc.highlight_power;
		}

		world.get_parent_scene()->get_render_scene().queue_draw(&model_instance, &transform, entity, highlight);
	}
}