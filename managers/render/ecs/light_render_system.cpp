#include "render/ecs/light_render_system.h"
#include "components/light_component.h"
#include "components/transform_component.h"
#include "cvars/cvars.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include "light_render_system.h"
#include <glm/ext/quaternion_common.hpp>

AutoCVarFloat cvar_light_threshold(
		"render.light_threshold", "At what point do we treat light intensity as 0", 0.025f, CVarFlags::EditFloatDrag);

AutoCVarInt cvar_light_debug_draw_enabled(
		"debug_draw.lights.draw", "enable lights debug draw", 1, CVarFlags::EditCheckbox);

void LightRenderSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<Light>());

	world.set_system_component_whitelist<LightRenderSystem>(whitelist);
}

void LightRenderSystem::update(World &world, float dt) {
	ZoneScopedN("LightRenderSystem::update");
	auto &render_scene = world.get_parent_scene()->get_render_scene();
	bool debug_draw_enabled = cvar_light_debug_draw_enabled.get() == 1;

	for (auto const &entity : entities) {
		auto &light = world.get_component<Light>(entity);
		auto &transform = world.get_component<Transform>(entity);

		if (light.is_on) {
			render_scene.queue_light_draw(&light, &transform);
		}

		if (!debug_draw_enabled) {
			continue;
		}
		const glm::vec3 &position = transform.get_position();
		const glm::quat &orientation = transform.get_global_orientation();
		const glm::vec3 &direction = glm::normalize(orientation * glm::vec3(0.0f, 0.0f, -1.0f));

		float radius = light.intensity * std::sqrtf(1.0f / cvar_light_threshold.get());
		float cone_radius;
		switch (light.type) {
			case LightType::DIRECTIONAL_LIGHT:
				render_scene.debug_draw.draw_arrow(position, position + direction * 2.0f, 1.0f, light.color, entity);
				break;
			case LightType::POINT_LIGHT:
				render_scene.debug_draw.draw_circle(position, glm::vec3(1.0f, 0.0f, 0.0f), radius, light.color, entity);
				render_scene.debug_draw.draw_circle(position, glm::vec3(0.0f, 1.0f, 0.0f), radius, light.color, entity);
				render_scene.debug_draw.draw_circle(position, glm::vec3(0.0f, 0.0f, 1.0f), radius, light.color, entity);
				break;
			case LightType::SPOT_LIGHT:
				cone_radius = radius * tan(glm::radians(light.cutoff + light.outer_cutoff));
				render_scene.debug_draw.draw_cone(
						position, position + direction, radius, cone_radius, light.color, entity, 4);
				break;
			default:
				SPDLOG_WARN("Invalid light type!");
				break;
		}
	}
}