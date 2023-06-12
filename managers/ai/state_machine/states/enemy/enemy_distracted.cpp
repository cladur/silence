#include "enemy_distracted.h"
#include "ai/state_machine/state_machine.h"
#include "components/enemy_data_component.h"
#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "engine/scene.h"
#include "managers/ecs/world.h"
#include "managers/render/render_scene.h"
#include <animation/animation_manager.h>
#include "enemy_utils.h"

void EnemyDistracted::startup(StateMachine *machine, std::string name) {
	SPDLOG_INFO("EnemyDistracted::startup");
	this->name = name;
	set_state_machine(machine);
}

void EnemyDistracted::enter() {
	SPDLOG_INFO("EnemyDistracted::enter");
	first_frame = true;
}

void EnemyDistracted::update(World *world, uint32_t entity_id, float dt) {
	auto &transform = world->get_component<Transform>(entity_id);
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &res = ResourceManager::get();
	auto &anim = world->get_component<AnimationInstance>(entity_id);
	auto &enemy_path = world->get_component<EnemyPath>(entity_id);
	auto &enemy_data = world->get_component<EnemyData>(entity_id);
	auto &dd = world->get_parent_scene()->get_render_scene().debug_draw;

	auto &current_pos = transform.position;
	auto &target = enemy_data.distraction_target;

	// move the entity towards the distraction target but keep it a bit away from it
	if (glm::distance(current_pos, target) > 1.0f) {

		// walking animation if walking
		if (anim.animation_handle.id != res.get_animation_handle("enemy/enemy_ANIM_GLTF/enemy_walk_with_gun.anim").id) {
			animation_manager.change_animation(entity_id, "enemy/enemy_ANIM_GLTF/enemy_walk_with_gun.anim");
		}

		transform.add_position(glm::normalize(target - current_pos) * enemy_path.speed * dt);

	} else {
		// idle animation if idle
		if (anim.animation_handle.id != res.get_animation_handle("enemy/enemy_ANIM_GLTF/enemy_idle.anim").id) {
			animation_manager.change_animation(entity_id, "enemy/enemy_ANIM_GLTF/enemy_idle.anim");
		}

		enemy_data.distraction_cooldown -= dt;
	}

	// smoothly rotate the entity to face the next node
	if (enemy_path.is_rotating) {
		float fast_dt = dt * 3.0f;
		enemy_utils::look_at(enemy_path, transform, target, fast_dt);
	}

	// if the entity is facing the next node, stop rotating
	if (glm::dot(
				glm::normalize(target - transform.position),
				glm::normalize(transform.get_global_forward())) > 0.99f) {
		enemy_path.is_rotating = false;
	} else {
		if (!enemy_path.is_rotating) {
			enemy_path.first_rotation_frame = true;
		}
		enemy_path.is_rotating = true;
	}

	enemy_utils::handle_detection(world, entity_id, transform, transform.get_global_forward(), enemy_data, dt, &dd);

	enemy_utils::update_detection_slider(entity_id, transform, enemy_data);

	enemy_utils::handle_highlight(entity_id, world);

	if (enemy_data.distraction_cooldown <= 0.0f) {
		// find the closes node to the entity
		auto &path = world->get_component<Children>(enemy_path.path_parent);
		uint32_t idx = enemy_utils::find_closest_node(world, current_pos, path);
		enemy_path.next_position = idx;
		enemy_path.prev_position = transform.position;
		enemy_path.is_rotating = true;
		state_machine->set_state("patrolling");
	}

	if (enemy_data.detection_level > 0.5f) {
		state_machine->set_state("looking");
	}
	first_frame = false;
}

void EnemyDistracted::exit() {
	SPDLOG_INFO("EnemyDistracted::exit");
}
