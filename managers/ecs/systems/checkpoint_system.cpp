#include "checkpoint_system.h"
#include "components/collider_obb.h"
#include "components/static_tag_component.h"
#include "components/transform_component.h"
#include "cvars/cvars.h"
#include "ecs/systems/dialogue_system.h"
#include "ecs/world.h"
#include "engine/scene.h"

#include "animation/animation_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "physics/physics_manager.h"

#include "input/input_manager.h"
#include "resource/resource_manager.h"
#include <audio/audio_manager.h>
#include <gameplay/gameplay_manager.h>
#include <render/transparent_elements/ui_manager.h>

void recursive_children(Entity entity, World &world, std::unordered_set<Entity> &entities_to_reset) {
	if (!world.has_component<Children>(entity)) {
		return;
	}

	auto &children = world.get_component<Children>(entity);
	auto &children_entities = world.get_component<Children>(entity).children;

	for (int i = 0; i < children.children_count; i++) {
		Entity child = children_entities[i];
		entities_to_reset.emplace(child);
		recursive_children(child, world, entities_to_reset);
	}
}

void CheckpointSystem::reset(World &world) {
	if (current_checkpoint == nullptr) {
		SPDLOG_WARN("Tried to reset to a checkpoint, but no checkpoint was set");
		return;
	}

	// Reset player's positions
	Entity agent = GameplayManager::get().get_agent_entity();
	Entity hacker = GameplayManager::get().get_hacker_entity();

	auto &agent_transform = world.get_component<Transform>(agent);
	auto &hacker_transform = world.get_component<Transform>(hacker);

	auto &agent_target_tf = world.get_component<Transform>(current_checkpoint->agent_spawn_pos);
	auto &hacker_target_tf = world.get_component<Transform>(current_checkpoint->hacker_spawn_pos);

	agent_transform = agent_target_tf;
	hacker_transform = hacker_target_tf;

	// Reset enemies
	std::ifstream file(world.get_parent_scene()->path);
	nlohmann::json serialized_scene = nlohmann::json::parse(file);
	file.close();

	world.deserialize_selected_entities_json(serialized_scene, current_checkpoint->entities_to_reset);
}

void CheckpointSystem::startup(World &world) {
	Signature blacklist;
	Signature whitelist;

	whitelist.set(world.get_component_type<Checkpoint>());

	world.set_system_component_whitelist<CheckpointSystem>(whitelist);
	world.set_system_component_blacklist<CheckpointSystem>(blacklist);

	auto &rm = ResourceManager::get();
}

void CheckpointSystem::update(World &world, float dt) {
	ZoneScopedN("CheckpointSystem::update");
	InputManager &input_manager = InputManager::get();
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &resource_manager = ResourceManager::get();
	PhysicsManager &physics_manager = PhysicsManager::get();

	// Wait for global matrices of transforms to be updated
	if (world.get_parent_scene()->frame_number == 0) {
		return;
	}

	if (input_manager.is_action_just_pressed("load_checkpoint")) {
		reset(world);
	}

	for (const Entity entity : entities) {
		auto &checkpoint = world.get_component<Checkpoint>(entity);

		if (checkpoint.player_collider == 0 || checkpoint.enemy_collider == 0 || checkpoint.agent_spawn_pos == 0 ||
				checkpoint.hacker_spawn_pos == 0) {
			SPDLOG_WARN("Checkpoint {} is missing some entity values", entity);
			continue;
		}

		if (!checkpoint.queried_enemies && world.get_parent_scene()->frame_number == 1) {
			checkpoint.queried_enemies = true;

			auto &enemy_collider = world.get_component<ColliderOBB>(checkpoint.enemy_collider);
			auto &enemy_collider_tf = world.get_component<Transform>(checkpoint.enemy_collider);

			ColliderOBB actual_enemy_collider = {};
			const glm::quat &orientation = enemy_collider_tf.get_global_orientation();

			actual_enemy_collider.set_orientation(orientation);
			actual_enemy_collider.range = enemy_collider.range * enemy_collider_tf.get_global_scale();
			actual_enemy_collider.center = enemy_collider_tf.get_global_position() +
					orientation * (enemy_collider.center * enemy_collider_tf.get_global_scale());

			std::vector<Entity> ent = PhysicsManager::get().overlap_cube_trigger(world, actual_enemy_collider);
			checkpoint.entities_to_reset = std::unordered_set<Entity>(ent.begin(), ent.end());

			for (auto entity : checkpoint.entities_to_reset) {
				recursive_children(entity, world, checkpoint.entities_to_reset);
			}
		}

		auto &player_collider = world.get_component<ColliderOBB>(checkpoint.player_collider);
		auto &player_collider_tf = world.get_component<Transform>(checkpoint.player_collider);

		if (checkpoint.reached) {
			continue;
		}

		ColliderOBB actual_player_collider = {};

		const glm::quat &orientation = player_collider_tf.get_global_orientation();
		actual_player_collider.set_orientation(orientation);
		actual_player_collider.range = player_collider.range * player_collider_tf.get_global_scale();
		actual_player_collider.center = player_collider_tf.get_global_position() +
				orientation * (player_collider.center * player_collider_tf.get_global_scale());

		auto players = PhysicsManager::get().overlap_cube_checkpoint(world, actual_player_collider);

		if (!players.empty()) {
			checkpoint.reached = true;
			current_checkpoint = &checkpoint;
		}
	}
}