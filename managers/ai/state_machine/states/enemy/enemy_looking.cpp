#include "enemy_looking.h"
#include "ai/state_machine/state_machine.h"
#include "components/enemy_data_component.h"
#include "components/enemy_path_component.h"
#include "components/transform_component.h"
#include "enemy_utils.h"
#include "engine/scene.h"
#include "managers/ecs/world.h"
#include "managers/gameplay/gameplay_manager.h"
#include "managers/physics/ecs/collision_system.h"
#include "managers/physics/physics_manager.h"
#include "managers/render/render_scene.h"
#include <animation/animation_manager.h>
#include <render/transparent_elements/ui_manager.h>

void EnemyLooking::startup(StateMachine *machine, std::string name) {
	SPDLOG_INFO("EnemyLooking::startup");
	this->name = name;
	set_state_machine(machine);
}

void EnemyLooking::enter() {
	first_frame = true;
	SPDLOG_INFO("EnemyLooking::enter");
}

void EnemyLooking::update(World *world, uint32_t entity_id, float dt) {
	auto &transform = world->get_component<Transform>(entity_id);
	AnimationManager &animation_manager = AnimationManager::get();
	ResourceManager &res = ResourceManager::get();
	auto &anim = world->get_component<AnimationInstance>(entity_id);
	auto &enemy_data = world->get_component<EnemyData>(entity_id);
	auto &dd = world->get_parent_scene()->get_render_scene().debug_draw;

	// some necessary variables
	auto agent_pos = GameplayManager::get().get_agent_position(world->get_parent_scene());
	auto forward = glm::normalize(transform.get_global_forward());
	auto current_no_y = glm::vec3(transform.position.x, 0.0f, transform.position.z);
	glm::vec3 target_look = glm::vec3(agent_pos.x, 0.0f, agent_pos.z);
	glm::vec3 direction = glm::normalize(target_look - current_no_y);
	glm::vec3 forward_no_y = glm::normalize(glm::vec3(forward.x, 0.0f, forward.z));

	if (first_frame) {
		float angle = glm::acos(glm::dot(forward_no_y, direction));
		glm::vec3 axis = glm::cross(forward_no_y, direction);
		rotation_end = (angle * axis);

		if (anim.animation_handle.id != res.get_animation_handle("enemy/enemy_ANIM_GLTF/enemy_down_to_aim.anim").id) {
			animation_manager.change_animation(entity_id, "enemy/enemy_ANIM_GLTF/enemy_down_to_aim.anim");
			anim.is_looping = false;
		}

		first_frame = false;
	}

	// smoothly look at the last known player position, stop if already looking
	float dot = glm::dot(glm::normalize(target_look - transform.position), forward);
	if (dot < 0.99f) {
		transform.add_global_euler_rot(rotation_end * dt * 2.0f);
	}
	dd.draw_line(transform.position + glm::vec3(0.0f, 0.1f, 0.0f), target_look, glm::vec3(1.0f, 0.0f, 0.0f));
	dd.draw_line(transform.position + glm::vec3(0.0f, 0.1f, 0.0f), transform.position + forward,
			glm::vec3(0.0f, 1.0f, 0.0f));

	enemy_utils::handle_detection(world, transform, enemy_data, dt, &dd);

	enemy_utils::update_detection_slider(entity_id, transform, enemy_data);

	if (enemy_data.detection_level < 0.2) {
		state_machine->set_state("patrolling");
	}
	if (enemy_data.detection_level > 0.99) {
		state_machine->set_state("fully_aware");
	}
}

void EnemyLooking::exit() {
	first_frame = true;
	SPDLOG_INFO("EnemyLooking::exit");
}
