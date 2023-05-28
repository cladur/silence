#include "collider_draw.h"
#include "engine/scene.h"

AutoCVarInt cvar_collider_draw_system_enabled(
		"debug_draw.collision.draw", "enable collider draw system", 0, CVarFlags::EditCheckbox);

void ColliderDrawSystem::startup(World &world) {
	Signature white_signature;
	white_signature.set(world.get_component_type<ColliderTag>());
	world.set_system_component_whitelist<ColliderDrawSystem>(white_signature);
}

void ColliderDrawSystem::update(World &world, float dt) {
	if (cvar_collider_draw_system_enabled.get() == 0) {
		return;
	}
	ZoneScopedN("ColliderDrawSystem::update");
	for (const Entity entity : entities) {
		const auto &transform = world.get_component<Transform>(entity);
		const glm::vec3 &position = transform.get_global_position();
		const glm::vec3 &scale = transform.get_global_scale();
		if (world.has_component<ColliderAABB>(entity)) {
			const auto &col = world.get_component<ColliderAABB>(entity);
			world.get_parent_scene()->get_render_scene().debug_draw.draw_box(
					position + col.center * scale, glm::vec3(0.0f), col.range * 2.0f * scale, glm::vec3(1.0f), entity);
		} else if (world.has_component<ColliderSphere>(entity)) {
			const auto &col = world.get_component<ColliderSphere>(entity);
			world.get_parent_scene()->get_render_scene().debug_draw.draw_sphere(
					position + col.center * scale, col.radius * scale.x, glm::vec3(1.0f), entity);
		} else if (world.has_component<ColliderOBB>(entity)) {
			const auto &col = world.get_component<ColliderOBB>(entity);
			const glm::quat &orientation = transform.get_global_orientation();
			world.get_parent_scene()->get_render_scene().debug_draw.draw_box(
					position + orientation * (col.center * scale), orientation, col.range * 2.0f * scale,
					glm::vec3(1.0f), entity);
		} else if (world.has_component<ColliderCapsule>(entity)) {
			const auto &col = world.get_component<ColliderCapsule>(entity);
			world.get_parent_scene()->get_render_scene().debug_draw.draw_sphere(
					position + col.start * scale, col.radius * scale.x, glm::vec3(1.0f), entity);
			world.get_parent_scene()->get_render_scene().debug_draw.draw_sphere(
					position + col.end * scale, col.radius * scale.x, glm::vec3(1.0f), entity);
			world.get_parent_scene()->get_render_scene().debug_draw.draw_line(
					position + col.start * scale, position + col.end * scale, glm::vec3(1.0f), entity);
		}
	}
}
