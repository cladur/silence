#include "light_system.h"
#include "components/light_component.h"
#include "components/transform_component.h"
#include "cvars/cvars.h"
#include "ecs/world.h"
#include "engine/scene.h"
#include <glm/ext/quaternion_common.hpp>

AutoCVarInt cvar_light_debug_draw_enabled(
		"debug_draw.lights.draw", "enable lights debug draw", 0, CVarFlags::EditCheckbox);

void LightSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Transform>());
	whitelist.set(world.get_component_type<Light>());

	world.set_system_component_whitelist<LightSystem>(whitelist);
}
void LightSystem::update(World &world, float dt) {
	int point_light_count = 0;
	auto &render_scene = world.get_parent_scene()->get_render_scene();
	auto &pbr_pass_shader = render_scene.pbr_pass.material.shader;
	pbr_pass_shader.use();

	bool debug_draw_enabled = cvar_light_debug_draw_enabled.get() == 1;

	for (auto const &entity : entities) {
		auto light_component = world.get_component<Light>(entity);
		auto transform_component = world.get_component<Transform>(entity);

		auto position = transform_component.get_position();
		auto direction = glm::vec3(0.0f, 0.0f, -1.0f);
		auto camera_pos = render_scene.camera_pos;
		auto model = transform_component.get_global_model_matrix();

		direction = glm::vec3(model * glm::vec4(direction, 1.0f));

		switch (light_component.type) {
			case LightType::DIRECTIONAL_LIGHT:
				if (debug_draw_enabled) {
					render_scene.debug_draw.draw_arrow(position, direction, 2.0f, light_component.get_color());
				}
				break;
			case LightType::POINT_LIGHT:
				if (point_light_count >= 32) {
					spdlog::warn("Too many point lights, max is 32");
					break;
				}
				if (debug_draw_enabled) {
					render_scene.debug_draw.draw_circle(position, direction, 0.5f, light_component.get_color());
				}
				camera_pos = render_scene.camera_pos;
				direction = camera_pos - position;
				pbr_pass_shader.set_vec3(fmt::format("lightPositions[{}]", point_light_count), position);
				pbr_pass_shader.set_vec3(
						fmt::format("lightColors[{}]", point_light_count), light_component.get_color());
				point_light_count++;
				break;
			case LightType::SPOT_LIGHT:
				if (debug_draw_enabled) {
					render_scene.debug_draw.draw_cone(position, direction, 2.0f, 1.0f, light_component.get_color());
				}
				break;
		}
	}
}