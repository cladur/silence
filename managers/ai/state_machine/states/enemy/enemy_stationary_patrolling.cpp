#include "enemy_stationary_patrolling.h"
#include "ai/state_machine/state_machine.h"
#include "components/enemy_data_component.h"
#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "engine/scene.h"
#include "managers/ecs/world.h"
#include "managers/render/render_scene.h"
#include <animation/animation_manager.h>
#include "enemy_utils.h"

void EnemyStationaryPatrolling::startup(StateMachine *machine, std::string name) {
	SPDLOG_INFO("EnemyStationaryPatrolling::startup");
	this->name = name;
	set_state_machine(machine);
}

void EnemyStationaryPatrolling::enter() {
	SPDLOG_INFO("EnemyStationaryPatrolling::enter");
}

void EnemyStationaryPatrolling::update(World *world, uint32_t entity_id, float dt) {
	auto &transform = world->get_component<Transform>(entity_id);
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &res = ResourceManager::get();
	auto &anim = world->get_component<AnimationInstance>(entity_id);
	auto &enemy_path = world->get_component<EnemyPath>(entity_id);
	auto &enemy_data = world->get_component<EnemyData>(entity_id);
	auto &dd = world->get_parent_scene()->get_render_scene().debug_draw;

	// change animation
	if (anim.animation_handle.id != res.get_animation_handle("enemy/enemy_ANIM_GLTF/enemy_idle.anim").id) {
		animation_manager.change_animation(entity_id, "enemy/enemy_ANIM_GLTF/enemy_idle.anim");
	}

	enemy_utils::handle_detection(world, transform, enemy_data, dt, &dd);

	enemy_utils::update_detection_slider(entity_id, transform, enemy_data);

	// decrease the cooldown
	enemy_path.patrol_cooldown -= dt;
	if (enemy_path.patrol_cooldown <= 0.0f) {
		enemy_path.patrol_cooldown = glm::max(0.0f, enemy_path.patrol_cooldown);
		enemy_path.is_patrolling = false;
		enemy_path.next_position = (enemy_path.next_position + 1) % enemy_path.path.size();
		state_machine->set_state("patrolling");
	}

	if (enemy_data.detection_level > 0.3f) {
		enemy_path.next_position = (enemy_path.next_position + 1) % enemy_path.path.size();
		state_machine->set_state("looking");
	}

}

void EnemyStationaryPatrolling::exit() {
	SPDLOG_INFO("EnemyStationaryPatrolling::exit");
}
