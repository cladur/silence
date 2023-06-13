#include "enemy_patrolling.h"
#include "ai/state_machine/state_machine.h"
#include "components/enemy_data_component.h"
#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "engine/scene.h"
#include "managers/ecs/world.h"
#include "managers/render/render_scene.h"
#include <animation/animation_manager.h>
#include "enemy_utils.h"

void EnemyPatrolling::startup(StateMachine *machine, std::string name) {
	SPDLOG_INFO("EnemyPatrolling::startup");
	this->name = name;
	set_state_machine(machine);
}

void EnemyPatrolling::enter() {
	SPDLOG_INFO("EnemyPatrolling::enter");
}

void EnemyPatrolling::update(World *world, uint32_t entity_id, float dt) {
	auto &transform = world->get_component<Transform>(entity_id);
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &res = ResourceManager::get();
	auto &anim = world->get_component<AnimationInstance>(entity_id);
	auto &enemy_path = world->get_component<EnemyPath>(entity_id);

	auto &path = world->get_component<Children>(enemy_path.path_parent);

	auto &enemy_data = world->get_component<EnemyData>(entity_id);
	auto &dd = world->get_parent_scene()->get_render_scene().debug_draw;

	if (first_frame_after_other_state) {
		first_frame_after_other_state = false;
		enemy_path.first_rotation_frame = true;
		enemy_path.is_rotating = true;
	}

	// change animation
	if (anim.animation_handle.id != res.get_animation_handle("enemy/enemy_ANIM_GLTF/enemy_walk_with_gun.anim").id) {
		animation_manager.change_animation(entity_id, "enemy/enemy_ANIM_GLTF/enemy_walk_with_gun.anim");
	}

	glm::vec3 current_position = transform.position;
	glm::vec3 target_position = world->get_component<Transform>(path.children[enemy_path.next_position]).get_global_position();

	// get index of previous node
	int idx = ((enemy_path.next_position - 1) % (int)path.children_count == -1)
			? path.children_count - 1
			: (enemy_path.next_position - 1) % path.children_count;

	auto &next_node = world->get_component<PathNode>(path.children[enemy_path.next_position]);
	auto &prev_node = world->get_component<PathNode>(path.children[idx]);

	enemy_path.prev_position = world->get_component<Transform>(path.children[idx]).get_global_position();

	// move the entity towards the next node
	if (glm::distance(current_position, target_position) > 0.1f) {
		transform.add_position(glm::normalize(target_position - current_position) * enemy_path.speed * dt);

		// if the point is a patrol point, switch state
	} else if (next_node.is_patrol_point) {
		enemy_path.patrol_cooldown = next_node.patrol_time;
		enemy_data.state_machine.set_state("stationary_patrolling");

		// if the node is not patrol node, just move to the next node
	} else {
		enemy_path.next_position = (enemy_path.next_position + 1) % path.children_count;
	}

	// this huge if just means "when near a node on either side" start rotating
	if (glm::distance(current_position, target_position) <
					(glm::distance(enemy_path.prev_position, target_position)) * 0.1f ||
			glm::distance(current_position, enemy_path.prev_position) <
					(glm::distance(enemy_path.prev_position, target_position)) * 0.1f) {
		if (!enemy_path.is_rotating) {
			enemy_path.first_rotation_frame = true;
		}
		enemy_path.is_rotating = true;
	}

	// smoothly rotate the entity to face the next node
	if (enemy_path.is_rotating) {
		float new_dt = dt * enemy_path.rotation_speed;
		enemy_utils::look_at(enemy_path, transform, target_position, new_dt);
	}

	// if the entity is facing the next node, stop rotating
	if (glm::dot(glm::normalize(target_position - transform.position), glm::normalize(transform.get_global_forward())) >
			0.99f) {
		enemy_path.is_rotating = false;
	}

	enemy_utils::handle_detection(world, entity_id, transform, transform.get_global_forward(), enemy_data, dt, &dd);

	enemy_utils::update_detection_slider(entity_id, transform, enemy_data);

	enemy_utils::handle_highlight(entity_id, world);

	if (enemy_data.detection_level > 0.3f) {
		state_machine->set_state("looking");
	}
}

void EnemyPatrolling::exit() {
	SPDLOG_INFO("EnemyPatrolling::exit");
}