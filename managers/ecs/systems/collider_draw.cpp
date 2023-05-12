#include "collider_draw.h"
#include "engine/scene.h"
#include <glm/gtx/quaternion.hpp>

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
	for (const Entity entity : entities) {
		if (world.has_component<ColliderAABB>(entity)) {
			auto col = world.get_component<ColliderAABB>(entity);
			auto transform = world.get_component<Transform>(entity);
			world.get_parent_scene()->get_render_scene().debug_draw.draw_box(transform.position + col.center,
					glm::vec3(0.0f, 0.0f, 0.0f), col.range * 2.0f * transform.scale, glm::vec3(1.0f, 1.0f, 1.0f));
		}

		if (world.has_component<ColliderSphere>(entity)) {
			auto col = world.get_component<ColliderSphere>(entity);
			auto transform = world.get_component<Transform>(entity);
			world.get_parent_scene()->get_render_scene().debug_draw.draw_sphere(
					transform.position + col.center, col.radius * transform.scale.x, glm::vec3(1.0f, 1.0f, 1.0f));
		}

		if (world.has_component<ColliderOBB>(entity)) {
			auto col = world.get_component<ColliderOBB>(entity);
			auto transform = world.get_component<Transform>(entity);
			world.get_parent_scene()->get_render_scene().debug_draw.draw_box(
					transform.position + glm::toMat3(transform.orientation) * (col.center * transform.scale),
					transform.orientation, col.range * 2.0f * transform.scale, glm::vec3(1.0f, 1.0f, 1.0f));
		}
	}
}
