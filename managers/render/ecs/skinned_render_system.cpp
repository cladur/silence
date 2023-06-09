#include "skinned_render_system.h"
#include "core/components/transform_component.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include "managers/render/render_manager.h"

void SkinnedRenderSystem::startup(World &world) {
	Signature signature;
	signature.set(world.get_component_type<Transform>());
	signature.set(world.get_component_type<SkinnedModelInstance>());
	world.set_system_component_whitelist<SkinnedRenderSystem>(signature);
}

void SkinnedRenderSystem::update(World &world, float dt) {
	ZoneScopedN("SkinnedRenderSystem::update");
	for (auto const &entity : entities) {
		auto &transform = world.get_component<Transform>(entity);
		auto &model_instance = world.get_component<SkinnedModelInstance>(entity);

		HighlightData highlight = {};

		if (world.has_component<Highlight>(entity)) {
			auto &hc = world.get_component<Highlight>(entity);
			highlight.highlighted = hc.highlighted;
			highlight.highlight_color = hc.highlight_color;
			highlight.target = hc.target;
			highlight.highlight_power = hc.highlight_power;
		}

		world.get_parent_scene()->get_render_scene().queue_skinned_draw(&model_instance, &transform, entity, highlight);
	}
}
