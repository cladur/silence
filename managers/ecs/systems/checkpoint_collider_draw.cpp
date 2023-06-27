#include "checkpoint_collider_draw.h"
#include "engine/scene.h"

AutoCVarInt cvar_checkpoint_collider_draw_system_enabled(
		"debug_draw.checkpoint_colliders.draw", "enable checkpoint collider draw system", 0, CVarFlags::EditCheckbox);

void CheckpointColliderDrawSystem::startup(World &world) {
	Signature white_signature;
	white_signature.set(world.get_component_type<Checkpoint>());
	world.set_system_component_whitelist<CheckpointColliderDrawSystem>(white_signature);
}

void CheckpointColliderDrawSystem::update(World &world, float dt) {
	if (cvar_checkpoint_collider_draw_system_enabled.get() == 0) {
		return;
	}
	ZoneScopedN("CheckpointColliderDrawSystem::update");
	for (const Entity entity : entities) {
		auto &checkpoint = world.get_component<Checkpoint>(entity);

		if (checkpoint.player_collider == 0 || checkpoint.enemy_collider == 0 || checkpoint.agent_spawn_pos == 0 ||
				checkpoint.hacker_spawn_pos == 0) {
			continue;
		}

		// Draw player_collider
		if (world.has_component<ColliderOBB>(checkpoint.player_collider)) {
			const auto &transform = world.get_component<Transform>(checkpoint.player_collider);
			const auto &col = world.get_component<ColliderOBB>(checkpoint.player_collider);

			const glm::vec3 &position = transform.get_global_position();
			const glm::vec3 &scale = transform.get_global_scale();
			const glm::quat &orientation = transform.get_global_orientation();

			world.get_parent_scene()->get_render_scene().debug_draw.draw_box(
					position + orientation * (col.center * scale), orientation, col.range * 2.0f * scale,
					glm::vec3(0.0f, 1.0f, 0.8f), checkpoint.player_collider);
		}

		// Draw enemy_collider
		if (world.has_component<ColliderOBB>(checkpoint.enemy_collider)) {
			const auto &transform = world.get_component<Transform>(checkpoint.enemy_collider);
			const auto &col = world.get_component<ColliderOBB>(checkpoint.enemy_collider);

			const glm::vec3 &position = transform.get_global_position();
			const glm::vec3 &scale = transform.get_global_scale();
			const glm::quat &orientation = transform.get_global_orientation();

			world.get_parent_scene()->get_render_scene().debug_draw.draw_box(
					position + orientation * (col.center * scale), orientation, col.range * 2.0f * scale,
					glm::vec3(1.0f, 0.0f, 0.8f), checkpoint.enemy_collider);
		}

		// Draw agent_spawn_pos
		{
			const auto &transform = world.get_component<Transform>(checkpoint.agent_spawn_pos);

			world.get_parent_scene()->get_render_scene().debug_draw.draw_box(transform.get_global_position(),
					glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(1.0f, 0.5f, 0.0f),
					checkpoint.agent_spawn_pos);
		}

		// Draw hacker_spawn_pos
		{
			const auto &transform = world.get_component<Transform>(checkpoint.hacker_spawn_pos);

			world.get_parent_scene()->get_render_scene().debug_draw.draw_box(transform.get_global_position(),
					glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 1.0f, 1.0f), glm::vec3(0.0f, 0.5f, 1.0f),
					checkpoint.hacker_spawn_pos);
		}
	}
}
